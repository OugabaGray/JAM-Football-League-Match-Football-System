#ifndef STANDING_H
#define STANDING_H

#include <string>

// One row of the league table. FootballLeague builds it from completed results.
class Standing {
private:
    std::string teamId;
    std::string teamName;
    int played;
    int wins;
    int draws;
    int losses;
    long long goalsFor;
    long long goalsAgainst;
    long long points;

public:
    Standing(const std::string& teamId, const std::string& teamName);
    void addMatch(int scored, int conceded, int pointsEarned);

    std::string getTeamId() const;
    std::string getTeamName() const;
    int getPlayed() const;
    int getWins() const;
    int getDraws() const;
    int getLosses() const;
    long long getGoalsFor() const;
    long long getGoalsAgainst() const;
    long long getGoalDifference() const;
    long long getPoints() const;
};

#endif
