#include "league/FootballLeague.h"
#include "league/LeagueCommissioner.h"
#include "league/LeagueRuleError.h"
#include "persistence/FileStore.h"
#include "reports/LeagueReports.h"

#include <iostream>
#include <sstream>
#include <string>

// Read one whole line so names and parishes can contain spaces.
static std::string readText(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    if (!std::getline(std::cin, input)) {
        throw LeagueRuleError("Input ended.");
    }
    return input;
}

// Read a line first, then check that it contains exactly one whole number.
// This lets the menu recover from bad input without leaving it in std::cin.
static int readNumber(const std::string& prompt) {
    std::istringstream input(readText(prompt));
    int number;
    char extra;
    if (!(input >> number) || input >> extra) {
        throw LeagueRuleError("Enter a whole number.");
    }
    return number;
}

static void showPerson(const Person& person) {
    // The same call works with any subclass of Person.
    person.display();
}

// The team supplies its data. The driver decides how to print it.
static void showTeam(const FootballTeam& team) {
    std::cout << team.getRegistrationNumber() << " | " << team.getName()
              << " | " << team.getParish() << " | ";
    switch (team.getStatus()) {
        case TeamStatus::Pending: std::cout << "Pending approval"; break;
        case TeamStatus::Registered: std::cout << "Registered"; break;
        case TeamStatus::Suspended: std::cout << "Suspended"; break;
    }
    std::cout << '\n';
}

static void showFixtures(const FootballLeague& league) {
    if (league.getFixtures().empty()) {
        std::cout << "No fixtures scheduled yet.\n";
    }
    for (const Fixture& fixture : league.getFixtures()) {
        std::cout << fixture.getId() << " | " << fixture.getDate()
                  << ' ' << fixture.getTime() << " | "
                  << league.getTeam(fixture.getHomeTeamId()).getName() << " vs "
                  << league.getTeam(fixture.getAwayTeamId()).getName() << " | "
                  << league.getStadium(fixture.getStadiumId()).getName();
        // A fixture only has a readable score after a result is recorded.
        if (fixture.isCompleted()) {
            const MatchResult& result = fixture.getResult();
            std::cout << " | " << result.getHomeScore() << '-' << result.getAwayScore();
        }
        std::cout << '\n';
    }
}

static void showStandings(const FootballLeague& league) {
    std::vector<Standing> table = league.calculateStandings();
    if (table.empty()) {
        std::cout << "No approved teams yet.\n";
        return;
    }
    std::cout << "Team | Played | Wins | Draws | Losses | GF | GA | GD | Points\n";
    for (const Standing& row : table) {
        std::cout << row.getTeamName() << " | " << row.getPlayed()
                  << " | " << row.getWins() << " | " << row.getDraws()
                  << " | " << row.getLosses() << " | " << row.getGoalsFor()
                  << " | " << row.getGoalsAgainst() << " | " << row.getGoalDifference()
                  << " | " << row.getPoints() << '\n';
    }
}

static void showMenu() {
    std::cout << "\nKingston Premier Football League\n"
              << " 1. Register team          2. List teams\n"
              << " 3. Find team              4. Update team\n"
              << " 5. Approve team           6. Suspend team\n"
              << " 7. Register stadium       8. Register official\n"
              << " 9. Schedule fixture      10. Assign official\n"
              << "11. Inspect fixture       12. Record score\n"
              << "13. List fixtures         14. Show standings\n"
              << "15. Save data             16. Load data\n"
              << "17. Reports               18. Postpone fixture\n"
              << "19. Official availability 20. Reserve seats\n"
              << "21. Add player            22. Update player\n"
              << "23. Remove player         24. Add coach\n"
              << "25. Update coach          26. Remove coach\n"
              << "27. Show squad            28. Search teams\n"
              << "29. Record player goals\n"
              << " 0. Exit\n";
}

// Each menu choice reads input and calls a class method. The class checks the rules.
static void runChoice(int choice, FootballLeague& league, const LeagueCommissioner& commissioner) {
    switch (choice) {
        case 1: {
            std::string id = readText("Registration number: ");
            std::string name = readText("Team name: ");
            std::string parish = readText("Parish: ");
            // The constructor creates a pending team. Option 5 handles approval.
            league.registerTeam(FootballTeam(id, name, parish));
            std::cout << "Team added. It needs commissioner approval.\n";
            break;
        }
        case 2:
            if (league.getTeams().empty()) {
                std::cout << "No teams registered yet.\n";
            }
            for (const FootballTeam* team : league.getTeams()) {
                showTeam(*team);
            }
            break;
        case 3:
            showTeam(league.getTeam(readText("Registration number: ")));
            break;
        case 4: {
            std::string id = readText("Registration number: ");
            std::string name = readText("New team name: ");
            std::string parish = readText("New parish: ");
            league.updateTeam(id, name, parish);
            std::cout << "Team updated.\n";
            break;
        }
        case 5:
            commissioner.approveRegistration(league, readText("Registration number: "));
            std::cout << "Team approved.\n";
            break;
        case 6:
            commissioner.suspendRegistration(league, readText("Registration number: "));
            std::cout << "Team suspended.\n";
            break;
        case 7: {
            std::string id = readText("Stadium ID: ");
            std::string name = readText("Stadium name: ");
            std::string location = readText("Location: ");
            int capacity = readNumber("Seating capacity: ");
            int condition = readNumber("Field condition (1 Excellent, 2 Good, 3 Fair, 4 Poor, 5 Unplayable): ");
            if (condition < 1 || condition > 5) {
                throw LeagueRuleError("Choose a field condition from 1 to 5.");
            }
            int price = readNumber("Ticket price in cents: ");
            league.registerStadium(Stadium(id, name, location, capacity,
                                           static_cast<FieldCondition>(condition - 1), price));
            std::cout << "Stadium added.\n";
            break;
        }
        case 8: {
            std::string id = readText("Official ID: ");
            std::string name = readText("Official name: ");
            int age = readNumber("Age: ");
            std::string gender = readText("Gender: ");
            std::string role = readText("Officiating role (e.g. Referee): ");
            std::string rank = readText("Rank (e.g. Grade 1): ");
            MatchOfficial official(id, name, age, gender, role, rank);
            league.registerOfficial(official);
            showPerson(official);
            break;
        }
        case 9: {
            std::string id = readText("Fixture ID: ");
            std::string home = readText("Home team registration number: ");
            std::string away = readText("Away team registration number: ");
            std::string stadium = readText("Stadium ID: ");
            std::string date = readText("Date (YYYY-MM-DD): ");
            std::string time = readText("Kickoff (HH:MM): ");
            league.scheduleFixture(id, home, away, stadium, date, time);
            std::cout << "Fixture scheduled.\n";
            break;
        }
        case 10: {
            std::string fixtureId = readText("Fixture ID: ");
            std::string officialId = readText("Official ID: ");
            commissioner.assignMatchOfficial(league, fixtureId, officialId);
            std::cout << "Official assigned.\n";
            break;
        }
        case 11: {
            std::string id = readText("Fixture ID: ");
            std::cout << commissioner.inspectMatchPreparations(league, id) << '\n';
            break;
        }
        case 12: {
            std::string id = readText("Fixture ID: ");
            int homeScore = readNumber("Home score: ");
            int awayScore = readNumber("Away score: ");
            // MatchResult checks the score before the league can store it.
            league.recordResult(id, MatchResult(homeScore, awayScore));
            std::cout << "Result recorded.\n";
            break;
        }
        case 13:
            showFixtures(league);
            break;
        case 14:
            showStandings(league);
            break;
        case 21: {
            std::string teamId = readText("Team registration number: ");
            std::string id = readText("Player ID: ");
            std::string name = readText("Player name: ");
            int age = readNumber("Age: ");
            std::string gender = readText("Gender: ");
            int jersey = readNumber("Jersey number: ");
            std::string position = readText("Position: ");
            league.addPlayer(teamId, Player(id, name, age, gender, jersey, position));
            std::cout << "Player added.\n";
            break;
        }
        case 22: {
            std::string teamId = readText("Team registration number: ");
            std::string id = readText("Player ID: ");
            std::string position = readText("New position: ");
            int jersey = readNumber("New jersey number: ");
            int status = readNumber("Status (1 Active, 2 Injured, 3 Suspended, 4 Retired): ");
            if (status < 1 || status > 4) {
                throw LeagueRuleError("Choose a status from 1 to 4.");
            }
            league.updatePlayer(teamId, id, position, jersey,
                                static_cast<PlayingStatus>(status - 1));
            std::cout << "Player updated.\n";
            break;
        }
        case 23: {
            std::string teamId = readText("Team registration number: ");
            league.removePlayer(teamId, readText("Player ID: "));
            std::cout << "Player removed.\n";
            break;
        }
        case 24: {
            std::string teamId = readText("Team registration number: ");
            std::string id = readText("Coach ID: ");
            std::string name = readText("Coach name: ");
            int age = readNumber("Age: ");
            std::string gender = readText("Gender: ");
            std::string qualification = readText("Qualification: ");
            std::string role = readText("Coaching role: ");
            league.addCoach(teamId, Coach(id, name, age, gender, qualification, role));
            std::cout << "Coach added.\n";
            break;
        }
        case 25: {
            std::string teamId = readText("Team registration number: ");
            std::string id = readText("Coach ID: ");
            std::string qualification = readText("New qualification: ");
            std::string role = readText("New coaching role: ");
            league.updateCoach(teamId, id, qualification, role);
            std::cout << "Coach updated.\n";
            break;
        }
        case 26: {
            std::string teamId = readText("Team registration number: ");
            league.removeCoach(teamId, readText("Coach ID: "));
            std::cout << "Coach removed.\n";
            break;
        }
        case 27: {
            const FootballTeam& team = league.getTeam(readText("Team registration number: "));
            showTeam(team);
            std::cout << "Coaches:\n";
            for (const Coach& coach : team.getCoaches()) {
                showPerson(coach);
            }
            std::cout << "Players (" << team.getSquadSize() << "):\n";
            for (const Player& player : team.getPlayers()) {
                showPerson(player);
            }
            break;
        }
        case 28: {
            std::vector<const FootballTeam*> found = league.searchTeams(readText("Search text: "));
            if (found.empty()) {
                std::cout << "No matching teams.\n";
            }
            for (const FootballTeam* team : found) {
                showTeam(*team);
            }
            break;
        }
        case 29: {
            std::string teamId = readText("Team registration number: ");
            std::string id = readText("Player ID: ");
            league.recordGoals(teamId, id, readNumber("Goals scored: "));
            std::cout << "Goals recorded.\n";
            break;
        }
        case 18:
            league.postponeFixture(readText("Fixture ID: "));
            std::cout << "Fixture postponed. Its team, stadium and officials are free again.\n";
            break;
        case 19: {
            std::string id = readText("Official ID: ");
            int available = readNumber("Available (1 for yes, 0 for no): ");
            if (available != 0 && available != 1) {
                throw LeagueRuleError("Enter 1 for yes or 0 for no.");
            }
            league.setOfficialAvailability(id, available == 1);
            std::cout << "Availability updated.\n";
            break;
        }
        case 20: {
            std::string id = readText("Stadium ID: ");
            int seats = readNumber("Seats to reserve: ");
            league.reserveSeats(id, seats);
            const Stadium& stadium = league.getStadium(id);
            std::cout << stadium.getCurrentOccupancy() << " of " << stadium.getSeatingCapacity()
                      << " seats taken.\n";
            break;
        }
        case 17: {
            // Show one report, then save it as text and CSV in data/reports/.
            std::cout << "1 Standings  2 Fixtures  3 Official assignments  4 Team status\n";
            int which = readNumber("Report: ");
            Report report;
            std::string file;
            if (which == 1) { report = LeagueReports::standings(league); file = "standings"; }
            else if (which == 2) { report = LeagueReports::fixtures(league); file = "fixtures"; }
            else if (which == 3) { report = LeagueReports::officialAssignments(league); file = "officials"; }
            else if (which == 4) { report = LeagueReports::teamStatus(league); file = "teams"; }
            else { throw LeagueRuleError("Choose a report from 1 to 4."); }
            ReportGenerator::print(report, std::cout);
            ReportGenerator::saveAsText(report, "data/reports/" + file + ".txt");
            ReportGenerator::saveAsCsv(report, "data/reports/" + file + ".csv");
            std::cout << "Saved to data/reports/" << file << ".txt and .csv\n";
            break;
        }
        case 15:
            FileStore::save(league);
            std::cout << "Data saved.\n";
            break;
        case 16:
            FileStore::load(league);
            std::cout << "Data loaded.\n";
            break;
        default:
            std::cout << "Choose a number from the menu.\n";
    }
}

int main() {
    // One league object holds the teams, fixtures, and results for this run.
    FootballLeague league;
    LeagueCommissioner commissioner("LC01", "Demo Commissioner", 35, "Unspecified");
    showPerson(commissioner);

    // Catch a rule error inside the loop so the user can try another option.
    while (true) {
        showMenu();
        try {
            int choice = readNumber("Choice: ");
            if (choice == 0) {
                break;
            }
            runChoice(choice, league, commissioner);
        } catch (const LeagueRuleError& error) {
            if (!std::cin) {
                break;
            }
            std::cout << "Error: " << error.what() << '\n';
        } catch (const FileException& error) {
            // File problems are reported without losing the data already in memory.
            std::cout << "File error: " << error.what() << '\n';
        }
    }
    return 0;
}
