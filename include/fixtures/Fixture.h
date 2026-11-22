#ifndef FIXTURE_H
#define FIXTURE_H

#include "fixtures/MatchOfficial.h"
#include "fixtures/Stadium.h"
#include "Persistable.h"
#include "league/FootballTeam.h"
#include "league/LeagueRuleError.h"
#include "league/MatchResult.h"
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

    static int readNumber(const std::string& text, int start, int count) {
        int number = 0;
        for (int i = start; i < start + count; i++) {
            if (text[i] < '0' || text[i] > '9') {
                throw LeagueRuleError("Use YYYY-MM-DD for the date and HH:MM for the time.");
            }
            number = number * 10 + (text[i] - '0');
        }
        return number;
    }

public:
    // Throws LeagueRuleError unless date is a real YYYY-MM-DD and time is a real HH:MM.
    static void checkKickoff(const std::string& date, const std::string& time) {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-' ||
            time.size() != 5 || time[2] != ':') {
            throw LeagueRuleError("Use YYYY-MM-DD for the date and HH:MM for the time.");
        }

        // Fixed positions work here because the separators and lengths were checked first.
        int year = readNumber(date, 0, 4);
        int month = readNumber(date, 5, 2);
        int day = readNumber(date, 8, 2);
        int hour = readNumber(time, 0, 2);
        int minute = readNumber(time, 3, 2);
        if (year < 1900 || month < 1 || month > 12 || hour > 23 || minute > 59) {
            throw LeagueRuleError("Enter a valid date and time.");
        }

        const int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int days = monthDays[month - 1];
        bool leapYear = year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
        if (month == 2 && leapYear) {
            days++;
        }
        if (day < 1 || day > days) {
            throw LeagueRuleError("That day does not exist in the selected month.");
        }
    }

    // The pointed-to objects must outlive the fixture. FootballLeague guarantees that.
    Fixture(const std::string& id, const FootballTeam* homeTeam, const FootballTeam* awayTeam,
            const Stadium* stadium, const std::string& date, const std::string& time)
        : id(id), homeTeam(homeTeam), awayTeam(awayTeam), stadium(stadium),
          date(date), time(time), result(0, 0), completed(false), postponed(false) {
        if (id.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw LeagueRuleError("A fixture needs an ID.");
        }
        if (homeTeam == nullptr || awayTeam == nullptr || stadium == nullptr) {
            throw LeagueRuleError("A fixture needs two teams and a stadium.");
        }
        if (homeTeam == awayTeam ||
            homeTeam->getRegistrationNumber() == awayTeam->getRegistrationNumber()) {
            throw LeagueRuleError("Choose two different teams.");
        }
        checkKickoff(date, time);
    }

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
    FixtureStatus getStatus() const {
        if (completed) {
            return FixtureStatus::Completed;
        }
        return postponed ? FixtureStatus::Postponed : FixtureStatus::Scheduled;
    }

    // A completed match cannot be postponed. Postponing twice is harmless.
    void postpone() {
        if (completed) {
            throw LeagueRuleError("A completed fixture cannot be postponed.");
        }
        postponed = true;
    }

    bool involvesTeam(const std::string& teamId) const {
        return teamId == getHomeTeamId() || teamId == getAwayTeamId();
    }

    bool hasOfficial(const std::string& officialId) const {
        for (const MatchOfficial* official : officials) {
            if (official->getId() == officialId) {
                return true;
            }
        }
        return false;
    }

    bool hasOfficials() const { return !officials.empty(); }

    const std::vector<const MatchOfficial*>& getOfficials() const { return officials; }

    void addOfficial(const MatchOfficial* official) {
        if (official == nullptr) {
            throw LeagueRuleError("Choose a match official.");
        }
        if (completed) {
            throw LeagueRuleError("You cannot assign officials to a completed fixture.");
        }
        if (postponed) {
            throw LeagueRuleError("You cannot assign officials to a postponed fixture.");
        }
        if (hasOfficial(official->getId())) {
            throw LeagueRuleError("That official is already assigned to this fixture.");
        }
        officials.push_back(official);
    }

    // Store the valid result and mark the fixture complete in the same operation.
    void recordResult(const MatchResult& newResult) {
        if (completed) {
            throw LeagueRuleError("This fixture already has a result.");
        }
        if (postponed) {
            throw LeagueRuleError("A postponed fixture cannot have a result.");
        }
        result = newResult;
        completed = true;
    }

    const MatchResult& getResult() const {
        if (!completed) {
            throw LeagueRuleError("This fixture has no result yet.");
        }
        return result;
    }

    // Saved as: id|homeTeamId|awayTeamId|stadiumId|date|time|officialIds|postponed (1 or 0)
    // officialIds is itself an escaped list. The result is saved separately (results file).
    std::string serialize() const override {
        std::vector<std::string> officialIds;
        for (const MatchOfficial* official : officials) {
            officialIds.push_back(official->getId());
        }
        return Codec::join({id, getHomeTeamId(), getAwayTeamId(), getStadiumId(), date, time,
                            Codec::join(officialIds), postponed ? "1" : "0"});
    }

    // A fixture holds pointers, so a record cannot build one alone. This holds the saved
    // values until the loader has found the objects the IDs refer to.
    struct Record {
        std::string id, homeTeamId, awayTeamId, stadiumId, date, time;
        std::vector<std::string> officialIds;
        bool postponed = false;
    };

    static Record fromRecord(const std::vector<std::string>& f) {
        Codec::requireFields(f, 8, "Fixture");
        Record record{f[0], f[1], f[2], f[3], f[4], f[5], {}, Codec::toBool(f[7])};
        if (!f[6].empty()) {
            record.officialIds = Codec::split(f[6]);
        }
        return record;
    }
};

#endif
