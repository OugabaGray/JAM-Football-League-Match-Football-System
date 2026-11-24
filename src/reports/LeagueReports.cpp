#include "reports/LeagueReports.h"

static std::string statusText(TeamStatus status) {
    switch (status) {
        case TeamStatus::Pending: return "Pending";
        case TeamStatus::Registered: return "Registered";
        default: return "Suspended";
    }
}

Report LeagueReports::standings(const FootballLeague& league) {
    Report report;
    report.title = "League Standings";
    report.headers = {"Pos", "Team", "P", "W", "D", "L", "GF", "GA", "GD", "Pts"};
    int position = 1;
    for (const Standing& row : league.calculateStandings()) {
        report.rows.push_back({std::to_string(position++), row.getTeamName(),
                               std::to_string(row.getPlayed()), std::to_string(row.getWins()),
                               std::to_string(row.getDraws()), std::to_string(row.getLosses()),
                               std::to_string(row.getGoalsFor()),
                               std::to_string(row.getGoalsAgainst()),
                               std::to_string(row.getGoalDifference()),
                               std::to_string(row.getPoints())});
    }
    report.footer = "Points: win 3, draw 1, loss 0. Ties: goal difference, goals scored, name.";
    return report;
}

Report LeagueReports::fixtures(const FootballLeague& league) {
    Report report;
    report.title = "Fixtures and Results";
    report.headers = {"ID", "Date", "Kickoff", "Home", "Away", "Stadium", "Officials", "Result"};
    int completed = 0;
    for (const Fixture& fixture : league.getFixtures()) {
        std::string result = fixture.isPostponed() ? "Postponed" : "Scheduled";
        if (fixture.isCompleted()) {
            result = std::to_string(fixture.getResult().getHomeScore()) + " - " +
                     std::to_string(fixture.getResult().getAwayScore());
            completed++;
        }
        report.rows.push_back({fixture.getId(), fixture.getDate(), fixture.getTime(),
                               fixture.getHomeTeam().getName(), fixture.getAwayTeam().getName(),
                               fixture.getStadium().getName(),
                               std::to_string(fixture.getOfficials().size()), result});
    }
    report.footer = "Completed: " + std::to_string(completed) + " of " +
                    std::to_string(league.getFixtures().size());
    return report;
}

Report LeagueReports::officialAssignments(const FootballLeague& league) {
    Report report;
    report.title = "Match Official Assignments";
    report.headers = {"Official ID", "Name", "Fixtures", "Fixture IDs", "Role", "Rank", "Available"};
    for (const MatchOfficial* official : league.getOfficials()) {
        int count = 0;
        std::string ids;
        for (const Fixture& fixture : league.getFixtures()) {
            if (fixture.hasOfficial(official->getId())) {
                count++;
                ids += (ids.empty() ? "" : " ") + fixture.getId();
            }
        }
        report.rows.push_back({official->getId(), official->getName(),
                               std::to_string(count), ids.empty() ? "-" : ids,
                               official->getOfficiatingRole(), official->getRank(),
                               official->isAvailable() ? "Yes" : "No"});
    }
    return report;
}

Report LeagueReports::teamStatus(const FootballLeague& league) {
    Report report;
    report.title = "Team Registration Status";
    report.headers = {"Reg No", "Team", "Parish", "Status", "Players", "Coaches"};
    int registered = 0;
    for (const FootballTeam* team : league.getTeams()) {
        report.rows.push_back({team->getRegistrationNumber(), team->getName(), team->getParish(),
                               statusText(team->getStatus()),
                               std::to_string(team->getSquadSize()),
                               std::to_string(team->getCoaches().size())});
        if (team->getStatus() == TeamStatus::Registered) {
            registered++;
        }
    }
    report.footer = "Registered teams: " + std::to_string(registered);
    return report;
}

Report LeagueReports::stadiumSummary(const FootballLeague& league) {
    Report report;
    report.title = "Stadium Summary";
    report.headers = {"ID", "Stadium", "Capacity", "Condition", "Ticket price", "Hosted", "Upcoming",
                      "Full-house gate"};
    for (const Stadium* stadium : league.getStadiums()) {
        int hosted = 0;
        int upcoming = 0;
        for (const Fixture& fixture : league.getFixtures()) {
            if (fixture.getStadium().getId() != stadium->getId()) {
                continue;
            }
            if (fixture.isCompleted()) {
                hosted++;
            } else if (!fixture.isPostponed()) {
                upcoming++;
            }
        }
        long long gate = static_cast<long long>(stadium->getSeatingCapacity()) *
                         stadium->getTicketPriceCents();
        report.rows.push_back({stadium->getId(), stadium->getName(),
                               std::to_string(stadium->getSeatingCapacity()),
                               fieldConditionName(stadium->getFieldCondition()),
                               ReportGenerator::moneyCents(stadium->getTicketPriceCents()),
                               std::to_string(hosted), std::to_string(upcoming),
                               ReportGenerator::moneyCents(gate)});
    }
    report.footer = "Full-house gate is capacity times ticket price (GCT included).";
    return report;
}
