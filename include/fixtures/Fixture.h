#ifndef FIXTURE_H
#define FIXTURE_H

#include "Persistable.h"
#include "fixtures/MatchResult.h"
#include "fixtures/Stadium.h"
#include "league/FootballTeam.h"
#include "personnel/MatchOfficial.h"
#include <string>
#include <vector>

// A postponed fixture keeps its record but no longer blocks teams, stadium or officials.
enum class FixtureStatus { Scheduled, Completed, Postponed };

// Relationships:
//  - Aggregation: the fixture points at two teams, a stadium and some officials. It does not
//    own them. They exist before the fixture and outlive it, and the league owns them.
//  - Composition: the fixture owns its MatchResult, which only exists as part of the fixture.
class Fixture : public Persistable {
private:
    std::string id;
    const FootballTeam* homeTeam;
    const FootballTeam* awayTeam;
    const Stadium* stadium;
    std::string date;
    std::string time;
    std::vector<const MatchOfficial*> officials;
    // completed tells us whether result contains an actual match score.
    // The initial 0-0 value is only a placeholder and cannot be read yet.
    MatchResult result;
    bool completed;
    bool postponed;

public:
    // Throws LeagueRuleError unless date is a real YYYY-MM-DD and time is a real HH:MM.
    static void checkKickoff(const std::string& date, const std::string& time);

    // The pointed-to objects must outlive the fixture. FootballLeague guarantees that.
    Fixture(const std::string& id, const FootballTeam* homeTeam, const FootballTeam* awayTeam,
            const Stadium* stadium, const std::string& date, const std::string& time);

    std::string getId() const { return id; }
    const FootballTeam& getHomeTeam() const { return *homeTeam; }
    const FootballTeam& getAwayTeam() const { return *awayTeam; }
    const Stadium& getStadium() const { return *stadium; }
    // The ID getters read through the links, so they always match the current objects.
    std::string getHomeTeamId() const { return homeTeam->getRegistrationNumber(); }
    std::string getAwayTeamId() const { return awayTeam->getRegistrationNumber(); }
    std::string getStadiumId() const { return stadium->getId(); }
    std::string getDate() const { return date; }
    std::string getTime() const { return time; }
    bool isCompleted() const { return completed; }
    bool isPostponed() const { return postponed; }
    FixtureStatus getStatus() const;

    // A completed match cannot be postponed. Postponing twice is harmless.
    void postpone();

    bool involvesTeam(const std::string& teamId) const;
    bool hasOfficial(const std::string& officialId) const;
    bool hasOfficials() const { return !officials.empty(); }
    const std::vector<const MatchOfficial*>& getOfficials() const { return officials; }
    void addOfficial(const MatchOfficial* official);

    // Store the valid result and mark the fixture complete in the same operation.
    void recordResult(const MatchResult& newResult);
    const MatchResult& getResult() const;

    // Saved as: id|homeTeamId|awayTeamId|stadiumId|date|time|officialIds|postponed (1 or 0)
    // officialIds is itself an escaped list. The result is saved separately (results file).
    std::string serialize() const override;

    // A fixture holds pointers, so a record cannot build one alone. This holds the saved
    // values until the loader has found the objects the IDs refer to.
    struct Record {
        std::string id, homeTeamId, awayTeamId, stadiumId, date, time;
        std::vector<std::string> officialIds;
        bool postponed = false;
    };

    static Record fromRecord(const std::vector<std::string>& f);
};

#endif
