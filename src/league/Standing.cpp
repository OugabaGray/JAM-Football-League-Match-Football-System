#include "league/Standing.h"

Standing::Standing(const std::string& teamId, const std::string& teamName)
    : teamId(teamId), teamName(teamName), played(0), wins(0), draws(0), losses(0),
      goalsFor(0), goalsAgainst(0), points(0) {}

// The league calls this once per completed fixture for each of its two teams.
void Standing::addMatch(int scored, int conceded, int pointsEarned) {
    played++;
    goalsFor += scored;
    goalsAgainst += conceded;
    points += pointsEarned;

    if (scored > conceded) {
        wins++;
    } else if (scored == conceded) {
        draws++;
    } else {
        losses++;
    }
}

std::string Standing::getTeamId() const { return teamId; }
std::string Standing::getTeamName() const { return teamName; }
int Standing::getPlayed() const { return played; }
int Standing::getWins() const { return wins; }
int Standing::getDraws() const { return draws; }
int Standing::getLosses() const { return losses; }
long long Standing::getGoalsFor() const { return goalsFor; }
long long Standing::getGoalsAgainst() const { return goalsAgainst; }
long long Standing::getGoalDifference() const { return goalsFor - goalsAgainst; }
long long Standing::getPoints() const { return points; }
