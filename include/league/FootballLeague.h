#ifndef FOOTBALL_LEAGUE_H
#define FOOTBALL_LEAGUE_H

#include "fixtures/Fixture.h"
#include "personnel/MatchOfficial.h"
#include "fixtures/Stadium.h"
#include "league/FootballTeam.h"
#include "fixtures/MatchResult.h"
#include "league/Standing.h"
#include <memory>
#include <string>
#include <vector>

// The league owns its teams, stadiums and officials, and checks rules that involve several
// of them. Each one lives on the heap behind a unique_ptr, so its address never changes when
// the list grows. That keeps the pointers stored in fixtures valid for the league's lifetime.
class FootballLeague {
private:
    std::vector<std::unique_ptr<FootballTeam>> teams;
    std::vector<std::unique_ptr<Stadium>> stadiums;
    std::vector<std::unique_ptr<MatchOfficial>> officials;
    std::vector<Fixture> fixtures;

    // A find method returns a list index, or -1 when the ID is missing.
    // A require method throws an error for a missing ID before we index the list.
    int findTeam(const std::string& teamId) const;
    int findStadium(const std::string& stadiumId) const;
    int findFixture(const std::string& fixtureId) const;
    int findOfficial(const std::string& officialId) const;
    int requireTeam(const std::string& teamId) const;
    int requireStadium(const std::string& stadiumId) const;
    int requireOfficial(const std::string& officialId) const;
    int requireFixture(const std::string& fixtureId) const;

    // FileStore rebuilds the private lists when it loads saved data.
    friend class FileStore;

public:
    FootballLeague() = default;
    // Copying would leave the copy's fixtures pointing at the original's objects.
    FootballLeague(const FootballLeague&) = delete;
    FootballLeague& operator=(const FootballLeague&) = delete;

    void registerTeam(const FootballTeam& team);
    void updateTeam(const std::string& teamId, const std::string& name,
                    const std::string& parish);
    void approveTeam(const std::string& teamId);
    void suspendTeam(const std::string& teamId);
    // const references let callers read the league's objects without editing them.
    const FootballTeam& getTeam(const std::string& teamId) const;
    std::vector<const FootballTeam*> getTeams() const;

    std::vector<const FootballTeam*> searchTeams(const std::string& text) const;
    void addPlayer(const std::string& teamId, const Player& player);
    void updatePlayer(const std::string& teamId, const std::string& playerId,
                      const std::string& position, int jerseyNumber, PlayingStatus status);
    void recordGoals(const std::string& teamId, const std::string& playerId, int goals);
    void removePlayer(const std::string& teamId, const std::string& playerId);
    void addCoach(const std::string& teamId, const Coach& coach);
    void updateCoach(const std::string& teamId, const std::string& coachId,
                     const std::string& qualification, const std::string& coachingRole);
    void removeCoach(const std::string& teamId, const std::string& coachId);

    void registerStadium(const Stadium& stadium);
    const Stadium& getStadium(const std::string& stadiumId) const;
    std::vector<const Stadium*> getStadiums() const;

    void registerOfficial(const MatchOfficial& official);
    const MatchOfficial& getOfficial(const std::string& officialId) const;
    std::vector<const MatchOfficial*> getOfficials() const;

    // The league looks the IDs up and links the new fixture to the stored objects.
    void scheduleFixture(const std::string& fixtureId, const std::string& homeTeamId,
                         const std::string& awayTeamId, const std::string& stadiumId,
                         const std::string& date, const std::string& time);
    void assignOfficial(const std::string& fixtureId, const std::string& officialId);
    void setOfficialAvailability(const std::string& officialId, bool available);
    // A postponed fixture stays on record but stops blocking teams, stadium and officials.
    void postponeFixture(const std::string& fixtureId);
    void updateFieldCondition(const std::string& stadiumId, FieldCondition condition);
    // Reserves seats at a stadium. Throws when the sale would pass the seating capacity.
    void reserveSeats(const std::string& stadiumId, int count);
    void recordResult(const std::string& fixtureId, const MatchResult& result);
    const Fixture& getFixture(const std::string& fixtureId) const;
    const std::vector<Fixture>& getFixtures() const;
    std::vector<Standing> calculateStandings() const;
};

#endif
