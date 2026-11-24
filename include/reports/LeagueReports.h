#ifndef LEAGUE_REPORTS_H
#define LEAGUE_REPORTS_H

#include "ReportGenerator.h"
#include "league/FootballLeague.h"

// Builds the league's management reports as plain tables for the shared ReportGenerator,
// which prints them or saves them as text or CSV. Nothing here changes the league.
class LeagueReports {
public:
    static Report standings(const FootballLeague& league);       // ranked league table
    static Report fixtures(const FootballLeague& league);        // every fixture with status and score
    static Report officialAssignments(const FootballLeague& league); // matches per official
    static Report teamStatus(const FootballLeague& league);      // registration status per team
    static Report stadiumSummary(const FootballLeague& league);  // capacity, price and fixtures per stadium
};

#endif
