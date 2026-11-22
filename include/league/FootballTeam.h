#ifndef FOOTBALL_TEAM_H
#define FOOTBALL_TEAM_H

#include "Persistable.h"
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

    int findPlayer(const std::string& playerId) const;
    int findCoach(const std::string& coachId) const;
    int requirePlayer(const std::string& playerId) const;
    int requireCoach(const std::string& coachId) const;
    bool jerseyTaken(int number, const std::string& exceptPlayerId) const;

public:
    FootballTeam(const std::string& registrationNumber, const std::string& name,
                 const std::string& parish);

    std::string getRegistrationNumber() const { return registrationNumber; }
    std::string getName() const { return name; }
    std::string getParish() const { return parish; }
    TeamStatus getStatus() const { return status; }

    void updateDetails(const std::string& newName, const std::string& newParish);
    void approveRegistration() { status = TeamStatus::Registered; }
    // Only approved teams can move from Registered to Suspended.
    void suspendRegistration();

    int getSquadSize() const { return static_cast<int>(players.size()); }
    const std::vector<Player>& getPlayers() const { return players; }
    const std::vector<Coach>& getCoaches() const { return coaches; }
    bool hasPlayer(const std::string& playerId) const { return findPlayer(playerId) != -1; }
    bool hasCoach(const std::string& coachId) const { return findCoach(coachId) != -1; }
    const Player& getPlayer(const std::string& playerId) const;
    const Coach& getCoach(const std::string& coachId) const;

    void addPlayer(const Player& player);
    void updatePlayer(const std::string& playerId, const std::string& position,
                      int jerseyNumber, PlayingStatus playingStatus);
    void recordGoals(const std::string& playerId, int goals);
    void removePlayer(const std::string& playerId);

    void addCoach(const Coach& coach);
    void updateCoach(const std::string& coachId, const std::string& qualification,
                     const std::string& coachingRole);
    void removeCoach(const std::string& coachId);

    // Saved as: registrationNumber|name|parish|status (0 pending, 1 registered, 2 suspended)
    std::string serialize() const override;
    static FootballTeam fromRecord(const std::vector<std::string>& f);
};

#endif
