#ifndef FOOTBALL_TEAM_H
#define FOOTBALL_TEAM_H

#include "Persistable.h"
#include "league/LeagueRuleError.h"
#include "personnel/Coach.h"
#include "personnel/Player.h"
#include <string>
#include <vector>

// A team starts pending, then the commissioner can register or suspend it.
enum class TeamStatus { Pending, Registered, Suspended };

class FootballTeam : public Persistable {
private:
    // Fixtures use this ID even if the team name or parish changes.
    std::string registrationNumber;
    std::string name;
    std::string parish;
    TeamStatus status;
    std::vector<Player> players;
    std::vector<Coach> coaches;

    int findPlayer(const std::string& playerId) const {
        for (std::size_t i = 0; i < players.size(); i++) {
            if (players[i].getId() == playerId) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    int findCoach(const std::string& coachId) const {
        for (std::size_t i = 0; i < coaches.size(); i++) {
            if (coaches[i].getId() == coachId) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    int requirePlayer(const std::string& playerId) const {
        int index = findPlayer(playerId);
        if (index == -1) {
            throw LeagueRuleError("Player not found: " + playerId);
        }
        return index;
    }

    int requireCoach(const std::string& coachId) const {
        int index = findCoach(coachId);
        if (index == -1) {
            throw LeagueRuleError("Coach not found: " + coachId);
        }
        return index;
    }

    bool jerseyTaken(int number, const std::string& exceptPlayerId) const {
        for (const Player& player : players) {
            if (player.getJerseyNumber() == number && player.getId() != exceptPlayerId) {
                return true;
            }
        }
        return false;
    }

public:
    FootballTeam(const std::string& registrationNumber, const std::string& name,
                 const std::string& parish)
        : registrationNumber(registrationNumber), status(TeamStatus::Pending) {
        if (registrationNumber.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw LeagueRuleError("A team needs a registration number.");
        }
        updateDetails(name, parish);
    }

    std::string getRegistrationNumber() const { return registrationNumber; }
    std::string getName() const { return name; }
    std::string getParish() const { return parish; }
    TeamStatus getStatus() const { return status; }

    void updateDetails(const std::string& newName, const std::string& newParish) {
        if (newName.find_first_not_of(" \t\r\n") == std::string::npos ||
            newParish.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw LeagueRuleError("A team needs a name and parish.");
        }
        name = newName;
        parish = newParish;
    }

    void approveRegistration() { status = TeamStatus::Registered; }

    // Only approved teams can move from Registered to Suspended.
    void suspendRegistration() {
        if (status != TeamStatus::Registered) {
            throw LeagueRuleError("Only a registered team can be suspended.");
        }
        status = TeamStatus::Suspended;
    }

    int getSquadSize() const { return static_cast<int>(players.size()); }
    const std::vector<Player>& getPlayers() const { return players; }
    const std::vector<Coach>& getCoaches() const { return coaches; }
    bool hasPlayer(const std::string& playerId) const { return findPlayer(playerId) != -1; }
    bool hasCoach(const std::string& coachId) const { return findCoach(coachId) != -1; }
    const Player& getPlayer(const std::string& playerId) const { return players[requirePlayer(playerId)]; }
    const Coach& getCoach(const std::string& coachId) const { return coaches[requireCoach(coachId)]; }

    void addPlayer(const Player& player) {
        if (hasPlayer(player.getId())) {
            throw LeagueRuleError("That player is already on this team.");
        }
        if (jerseyTaken(player.getJerseyNumber(), "")) {
            throw LeagueRuleError("Jersey number " + std::to_string(player.getJerseyNumber()) +
                                  " is already taken on this team.");
        }
        players.push_back(player);
    }

    void updatePlayer(const std::string& playerId, const std::string& position,
                      int jerseyNumber, PlayingStatus playingStatus) {
        int index = requirePlayer(playerId);
        if (jerseyTaken(jerseyNumber, playerId)) {
            throw LeagueRuleError("Jersey number " + std::to_string(jerseyNumber) +
                                  " is already taken on this team.");
        }
        Player updated = players[index];
        updated.setPosition(position);
        updated.setJerseyNumber(jerseyNumber);
        updated.setPlayingStatus(playingStatus);
        players[index] = updated;
    }

    void recordGoals(const std::string& playerId, int goals) {
        players[requirePlayer(playerId)].addGoals(goals);
    }

    void removePlayer(const std::string& playerId) {
        players.erase(players.begin() + requirePlayer(playerId));
    }

    void addCoach(const Coach& coach) {
        if (hasCoach(coach.getId())) {
            throw LeagueRuleError("That coach is already on this team.");
        }
        Coach assigned = coach;
        assigned.assignToTeam(registrationNumber);
        coaches.push_back(assigned);
    }

    void updateCoach(const std::string& coachId, const std::string& qualification,
                     const std::string& coachingRole) {
        int index = requireCoach(coachId);
        Coach updated = coaches[index];
        updated.setQualification(qualification);
        updated.setCoachingRole(coachingRole);
        coaches[index] = updated;
    }

    void removeCoach(const std::string& coachId) {
        coaches.erase(coaches.begin() + requireCoach(coachId));
    }

    // Saved as: registrationNumber|name|parish|status (0 pending, 1 registered, 2 suspended)
    std::string serialize() const override {
        return Codec::join({registrationNumber, name, parish,
                            std::to_string(static_cast<int>(status))});
    }

    static FootballTeam fromRecord(const std::vector<std::string>& f) {
        Codec::requireFields(f, 4, "FootballTeam");
        FootballTeam team(f[0], f[1], f[2]);
        int code = Codec::toInt(f[3], "team status");
        if (code < 0 || code > 2) {
            throw DataFormatException("team status " + f[3] + " is not 0, 1 or 2");
        }
        // Rebuild the status through the normal methods so the rules still apply.
        if (code >= 1) {
            team.approveRegistration();
        }
        if (code == 2) {
            team.suspendRegistration();
        }
        return team;
    }
};

#endif
