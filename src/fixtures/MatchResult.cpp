#include "fixtures/MatchResult.h"
#include "league/LeagueRuleError.h"

MatchResult::MatchResult(int homeScore, int awayScore)
    : homeScore(homeScore), awayScore(awayScore) {
    // Reject the whole object if either score is invalid.
    if (homeScore < 0 || awayScore < 0) {
        throw LeagueRuleError("Match scores cannot be negative.");
    }
}

std::string MatchResult::serialize() const {
    return Codec::join({std::to_string(homeScore), std::to_string(awayScore)});
}

MatchResult MatchResult::fromRecord(const std::vector<std::string>& fields) {
    Codec::requireFields(fields, 2, "MatchResult");
    // The constructor still rejects a negative score in a damaged file.
    return MatchResult(Codec::toInt(fields[0], "home score"),
                       Codec::toInt(fields[1], "away score"));
}

int MatchResult::getHomeScore() const {
    return homeScore;
}

int MatchResult::getAwayScore() const {
    return awayScore;
}

// Each side earns 3 for a win, 1 for a draw, or 0 for a loss.
int MatchResult::getHomePoints() const {
    if (homeScore > awayScore) {
        return 3;
    }
    if (homeScore == awayScore) {
        return 1;
    }
    return 0;
}

int MatchResult::getAwayPoints() const {
    if (awayScore > homeScore) {
        return 3;
    }
    if (awayScore == homeScore) {
        return 1;
    }
    return 0;
}
