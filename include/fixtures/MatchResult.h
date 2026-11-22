#ifndef MATCH_RESULT_H
#define MATCH_RESULT_H

#include "Persistable.h"
#include <string>
#include <vector>

// This class keeps a valid score and works out league points from it.
class MatchResult : public Persistable {
private:
    int homeScore;
    int awayScore;

public:
    MatchResult(int homeScore, int awayScore);

    // These accessors are const because reading a score or points changes nothing.
    int getHomeScore() const;
    int getAwayScore() const;
    int getHomePoints() const;
    int getAwayPoints() const;

    // Saved as: homeScore|awayScore. The store adds the fixture ID in front.
    std::string serialize() const override;
    static MatchResult fromRecord(const std::vector<std::string>& fields);
};

#endif
