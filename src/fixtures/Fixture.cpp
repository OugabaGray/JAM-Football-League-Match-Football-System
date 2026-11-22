#include "fixtures/Fixture.h"
#include "league/LeagueRuleError.h"

namespace {
int readNumber(const std::string& text, int start, int count) {
    int number = 0;
    for (int i = start; i < start + count; i++) {
        if (text[i] < '0' || text[i] > '9') {
            throw LeagueRuleError("Use YYYY-MM-DD for the date and HH:MM for the time.");
        }
        number = number * 10 + (text[i] - '0');
    }
    return number;
}
}

void Fixture::checkKickoff(const std::string& date, const std::string& time) {
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

Fixture::Fixture(const std::string& id, const FootballTeam* homeTeam,
                 const FootballTeam* awayTeam, const Stadium* stadium,
                 const std::string& date, const std::string& time)
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

FixtureStatus Fixture::getStatus() const {
    if (completed) {
        return FixtureStatus::Completed;
    }
    return postponed ? FixtureStatus::Postponed : FixtureStatus::Scheduled;
}

void Fixture::postpone() {
    if (completed) {
        throw LeagueRuleError("A completed fixture cannot be postponed.");
    }
    postponed = true;
}

bool Fixture::involvesTeam(const std::string& teamId) const {
    return teamId == getHomeTeamId() || teamId == getAwayTeamId();
}

bool Fixture::hasOfficial(const std::string& officialId) const {
    for (const MatchOfficial* official : officials) {
        if (official->getId() == officialId) {
            return true;
        }
    }
    return false;
}

void Fixture::addOfficial(const MatchOfficial* official) {
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

void Fixture::recordResult(const MatchResult& newResult) {
    if (completed) {
        throw LeagueRuleError("This fixture already has a result.");
    }
    if (postponed) {
        throw LeagueRuleError("A postponed fixture cannot have a result.");
    }
    result = newResult;
    completed = true;
}

const MatchResult& Fixture::getResult() const {
    if (!completed) {
        throw LeagueRuleError("This fixture has no result yet.");
    }
    return result;
}

std::string Fixture::serialize() const {
    std::vector<std::string> officialIds;
    for (const MatchOfficial* official : officials) {
        officialIds.push_back(official->getId());
    }
    return Codec::join({id, getHomeTeamId(), getAwayTeamId(), getStadiumId(), date, time,
                        Codec::join(officialIds), postponed ? "1" : "0"});
}

Fixture::Record Fixture::fromRecord(const std::vector<std::string>& f) {
    Codec::requireFields(f, 8, "Fixture");
    Record record{f[0], f[1], f[2], f[3], f[4], f[5], {}, Codec::toBool(f[7])};
    if (!f[6].empty()) {
        record.officialIds = Codec::split(f[6]);
    }
    return record;
}
