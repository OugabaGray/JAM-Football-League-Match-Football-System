#include "league/FootballLeague.h"
#include "personnel/LeagueCommissioner.h"
#include "league/LeagueRuleError.h"
#include "persistence/FileStore.h"
#include "FileManager.h"
#include "reports/LeagueReports.h"
#include "personnel/Spectator.h"
#include "sales/FoodItem.h"
#include "sales/LeagueSales.h"
#include "sales/Merchandise.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static int passed = 0;
static int failed = 0;

static void check(bool condition, const std::string& testName) {
    if (condition) {
        passed++;
        std::cout << "PASS: " << testName << '\n';
    } else {
        failed++;
        std::cout << "FAIL: " << testName << '\n';
    }
}

static void testScores() {
    // Test all three outcomes, then confirm that the constructor rejects a negative score.
    MatchResult homeWin(2, 0);
    MatchResult awayWin(0, 2);
    MatchResult draw(0, 0);
    check(homeWin.getHomePoints() == 3 && homeWin.getAwayPoints() == 0, "Home win gives 3/0");
    check(awayWin.getHomePoints() == 0 && awayWin.getAwayPoints() == 3, "Away win gives 0/3");
    check(draw.getHomePoints() == 1 && draw.getAwayPoints() == 1, "Zero-score draw gives 1/1");
    try {
        MatchResult invalid(-1, 0);
        check(false, "Reject a negative score");
    } catch (const LeagueRuleError&) {
        check(true, "Reject a negative score");
    }
}

static void testPolymorphism(const Person& commissioner, const Person& official) {
    // Capture the printed roles so we can check which display methods were called.
    std::ostringstream output;
    std::streambuf* original = std::cout.rdbuf(output.rdbuf());
    commissioner.display();
    official.display();
    std::cout.rdbuf(original);
    check(output.str().find("League Commissioner") != std::string::npos &&
          output.str().find("Match Official") != std::string::npos,
          "Person references call two different display methods");
}

static void testLeague() {
    FootballLeague league;
    LeagueCommissioner commissioner("LC1", "Demo Commissioner", 35, "Unspecified");
    MatchOfficial official("R1", "Demo Referee", 32, "Unspecified");
    testPolymorphism(commissioner, official);
    league.registerOfficial(official);
    league.registerTeam(FootballTeam("A", "Harbour FC", "Kingston"));
    league.registerTeam(FootballTeam("B", "Metro FC", "St Andrew"));
    league.registerTeam(FootballTeam("C", "Cedar FC", "St Catherine"));
    league.registerTeam(FootballTeam("D", "Valley FC", "St Thomas"));
    league.registerStadium(Stadium("S1", "East Park", "Kingston", 100, FieldCondition::Good, 500));
    league.registerStadium(Stadium("S2", "West Park", "St Andrew", 100, FieldCondition::Good, 500));

    league.updateTeam("A", "Harbour United", "Kingston");
    check(league.getTeam("A").getName() == "Harbour United", "Find and update a team by ID");
    try {
        league.registerTeam(FootballTeam("A", "Duplicate", "Kingston"));
        check(false, "Reject duplicate team ID");
    } catch (const LeagueRuleError&) {
        check(true, "Reject duplicate team ID");
    }

    try {
        league.scheduleFixture("F1", "A", "B", "S1", "2026-11-12", "15:00");
        check(false, "Reject a fixture with pending teams");
    } catch (const LeagueRuleError&) {
        check(league.getFixtures().empty(), "Reject a fixture with pending teams");
    }

    // After approval, the same fixture that failed earlier should be schedulable.
    commissioner.approveRegistration(league, "A");
    commissioner.approveRegistration(league, "B");
    commissioner.approveRegistration(league, "C");
    commissioner.approveRegistration(league, "D");
    league.scheduleFixture("F1", "A", "B", "S1", "2026-11-12", "15:00");
    check(!league.getFixture("F1").isCompleted(), "A new fixture starts without a result");
    check(commissioner.inspectMatchPreparations(league, "F1") ==
          "Assign at least one match official before kickoff.", "Inspection detects missing official");

    try {
        league.scheduleFixture("F2", "A", "C", "S2", "2026-11-12", "15:00");
        check(false, "Reject a team scheduling conflict");
    } catch (const LeagueRuleError&) {
        check(league.getFixtures().size() == 1, "Reject a team scheduling conflict");
    }
    try {
        league.scheduleFixture("F2", "C", "D", "S1", "2026-11-12", "15:00");
        check(false, "Reject a stadium scheduling conflict");
    } catch (const LeagueRuleError&) {
        check(league.getFixtures().size() == 1, "Reject a stadium scheduling conflict");
    }

    league.scheduleFixture("F2", "C", "D", "S2", "2026-11-12", "15:00");
    try {
        commissioner.assignMatchOfficial(league, "F1", "UNKNOWN");
        check(false, "Reject an unregistered official");
    } catch (const LeagueRuleError&) {
        check(!league.getFixture("F1").hasOfficials(), "Reject an unregistered official");
    }
    commissioner.assignMatchOfficial(league, "F1", "R1");
    check(commissioner.inspectMatchPreparations(league, "F1") ==
          "Teams, stadium and official assignment are ready.", "Inspection confirms readiness");
    try {
        commissioner.assignMatchOfficial(league, "F2", "R1");
        check(false, "Reject an official scheduling conflict");
    } catch (const LeagueRuleError&) {
        check(!league.getFixture("F2").hasOfficials(), "Reject an official scheduling conflict");
    }

    commissioner.suspendRegistration(league, "A");
    check(commissioner.inspectMatchPreparations(league, "F1") ==
          "Both teams must remain registered.", "Inspection sees a later suspension");
    try {
        league.scheduleFixture("F3", "A", "B", "S1", "2026-11-12", "16:00");
        check(false, "Reject scheduling a suspended team");
    } catch (const LeagueRuleError&) {
        check(league.getFixtures().size() == 2, "Reject scheduling a suspended team");
    }
    commissioner.approveRegistration(league, "A");

    try {
        league.recordResult("F1", MatchResult(-1, 0));
        check(false, "An invalid score cannot complete the fixture");
    } catch (const LeagueRuleError&) {
        check(!league.getFixture("F1").isCompleted(), "An invalid score cannot complete the fixture");
    }
    league.recordResult("F1", MatchResult(2, 0));
    check(league.getFixture("F1").isCompleted() &&
          league.getFixture("F1").getResult().getHomeScore() == 2, "A valid score completes the fixture");
    try {
        league.recordResult("F1", MatchResult(1, 1));
        check(false, "Reject a second result");
    } catch (const LeagueRuleError&) {
        check(league.getFixture("F1").getResult().getHomeScore() == 2, "Reject a second result");
    }

    std::vector<Standing> table = league.calculateStandings();
    check(table.size() == 4 && table[0].getTeamId() == "A" &&
          table[0].getPoints() == 3 && table[0].getWins() == 1 &&
          table[0].getGoalDifference() == 2, "Standings count the win and rank the winner first");
    check(table.size() == 4 && table[3].getTeamId() == "B" &&
          table[3].getPoints() == 0 && table[3].getLosses() == 1,
          "Standings count the loss");
    // Reading the standings a second time should still count just one match.
    table = league.calculateStandings();
    check(table.size() == 4 && table[0].getPoints() == 3 &&
          table[0].getPlayed() == 1, "Reading standings again does not count the match twice");

    league.scheduleFixture("F3", "A", "B", "S1", "2028-02-29", "23:59");
    check(league.getFixture("F3").getDate() == "2028-02-29",
          "Accept a leap day and the latest valid kickoff");
    try {
        league.scheduleFixture("F4", "A", "B", "S1", "2026-02-29", "10:00");
        check(false, "Reject an invalid calendar date");
    } catch (const LeagueRuleError&) {
        check(league.getFixtures().size() == 3, "Reject an invalid calendar date");
    }
}

// Fixtures hold pointers to the league's objects, so check those links stay safe and live.
static void testLinks() {
    FootballLeague league;
    league.registerTeam(FootballTeam("A", "Harbour FC", "Kingston"));
    league.registerTeam(FootballTeam("B", "Metro FC", "St Andrew"));
    league.registerStadium(Stadium("S1", "East Park", "Kingston", 100, FieldCondition::Good, 500));
    league.approveTeam("A");
    league.approveTeam("B");
    league.scheduleFixture("F1", "A", "B", "S1", "2026-11-12", "15:00");

    // Growing every list would move the elements of a plain vector. The links must survive.
    for (int i = 0; i < 200; i++) {
        std::string number = std::to_string(i);
        league.registerTeam(FootballTeam("T" + number, "Team " + number, "Kingston"));
        league.registerStadium(Stadium("X" + number, "Park " + number, "Kingston", 50, FieldCondition::Good, 500));
        league.registerOfficial(MatchOfficial("O" + number, "Official " + number, 30, "F"));
    }
    check(league.getFixture("F1").getHomeTeam().getName() == "Harbour FC" &&
          league.getFixture("F1").getStadium().getName() == "East Park",
          "Fixture links stay valid after the league lists grow");

    // The fixture points at the stored team, so an update shows through without a lookup.
    league.updateTeam("A", "Harbour United", "Kingston");
    check(league.getFixture("F1").getHomeTeam().getName() == "Harbour United",
          "A team update is visible through the fixture's link");

    league.assignOfficial("F1", "O0");
    check(league.getFixture("F1").getOfficials().size() == 1 &&
          league.getFixture("F1").getOfficials()[0] == &league.getOfficial("O0"),
          "An assigned official is the league's stored object");

    // The constructor refuses missing objects and a team playing itself.
    FootballTeam team("Z", "Zed FC", "Kingston");
    Stadium stadium("Z1", "Zed Park", "Kingston", 10, FieldCondition::Good, 500);
    try {
        Fixture noStadium("F9", &team, &team, nullptr, "2026-11-12", "15:00");
        check(false, "Reject a fixture with a missing link");
    } catch (const LeagueRuleError&) {
        check(true, "Reject a fixture with a missing link");
    }
    try {
        Fixture sameTeam("F9", &team, &team, &stadium, "2026-11-12", "15:00");
        check(false, "Reject a team playing itself");
    } catch (const LeagueRuleError&) {
        check(true, "Reject a team playing itself");
    }
    try {
        league.scheduleFixture("F9", "A", "NOPE", "S1", "2026-11-12", "17:00");
        check(false, "Reject a fixture with an unknown team ID");
    } catch (const LeagueRuleError&) {
        check(league.getFixtures().size() == 1, "Reject a fixture with an unknown team ID");
    }
}

// Stadium occupancy, official availability and postponement (ported from Section C).
static void testVenueAndOfficials() {
    // Seats: the boundary is exactly the capacity, and one more must be refused.
    Stadium small("S9", "Tiny Park", "Kingston", 10, FieldCondition::Fair, 750);
    small.bookSeats(10);
    check(small.getSeatsLeft() == 0 && small.getCurrentOccupancy() == 10,
          "Booking exactly the capacity is accepted");
    try {
        small.bookSeats(1);
        check(false, "Reject a booking over capacity");
    } catch (const LeagueRuleError&) {
        check(small.getCurrentOccupancy() == 10, "Reject a booking over capacity");
    }
    try {
        small.resetOccupancy();
        small.bookSeats(0);
        check(false, "Reject a zero-seat booking");
    } catch (const LeagueRuleError&) {
        check(true, "Reject a zero-seat booking");
    }
    try {
        Stadium bad("S8", "Bad", "Kingston", 10, FieldCondition::Good, -1);
        check(false, "Reject a negative ticket price");
    } catch (const LeagueRuleError&) {
        check(true, "Reject a negative ticket price");
    }
    check(!Stadium("S7", "Mud", "Kingston", 10, FieldCondition::Unplayable, 0).isPlayable() &&
          Stadium("S6", "Grass", "Kingston", 10, FieldCondition::Poor, 0).isPlayable(),
          "Only an unplayable field blocks scheduling");

    FootballLeague league;
    league.registerTeam(FootballTeam("A", "Harbour FC", "Kingston"));
    league.registerTeam(FootballTeam("B", "Metro FC", "St Andrew"));
    league.registerTeam(FootballTeam("C", "Cedar FC", "St Catherine"));
    league.registerTeam(FootballTeam("D", "Valley FC", "St Thomas"));
    for (const char* id : {"A", "B", "C", "D"}) {
        league.approveTeam(id);
    }
    league.registerStadium(Stadium("S1", "East Park", "Kingston", 50, FieldCondition::Good, 500));
    league.registerStadium(Stadium("S2", "Mud Park", "Kingston", 50, FieldCondition::Unplayable, 500));
    league.registerOfficial(MatchOfficial("R1", "Ref One", 32, "F", "Referee", "Grade 1"));

    league.reserveSeats("S1", 50);
    try {
        league.reserveSeats("S1", 1);
        check(false, "The league rejects tickets over a stadium's capacity");
    } catch (const LeagueRuleError&) {
        check(league.getStadium("S1").getCurrentOccupancy() == 50,
              "The league rejects tickets over a stadium's capacity");
    }
    try {
        league.scheduleFixture("F0", "A", "B", "S2", "2026-12-01", "15:00");
        check(false, "Reject a fixture at an unplayable stadium");
    } catch (const LeagueRuleError&) {
        check(league.getFixtures().empty(), "Reject a fixture at an unplayable stadium");
    }
    league.updateFieldCondition("S2", FieldCondition::Good);
    league.scheduleFixture("F0", "A", "B", "S2", "2026-12-01", "15:00");
    check(league.getFixtures().size() == 1, "A repaired field can host a fixture");

    league.scheduleFixture("F1", "A", "B", "S1", "2026-12-08", "15:00");
    league.setOfficialAvailability("R1", false);
    try {
        league.assignOfficial("F1", "R1");
        check(false, "Reject an unavailable official");
    } catch (const LeagueRuleError&) {
        check(!league.getFixture("F1").hasOfficials(), "Reject an unavailable official");
    }
    league.setOfficialAvailability("R1", true);
    league.assignOfficial("F1", "R1");
    league.setOfficialAvailability("R1", false);
    LeagueCommissioner commissioner("LC1", "Commissioner", 40, "F");
    check(commissioner.inspectMatchPreparations(league, "F1") ==
          "An assigned match official is no longer available.",
          "Inspection notices an official who became unavailable");

    // Postponing frees the slot, blocks results and officials, and cannot undo a result.
    try {
        league.scheduleFixture("F2", "A", "C", "S1", "2026-12-08", "15:00");
        check(false, "A live fixture blocks the same team and time");
    } catch (const LeagueRuleError&) {
        check(true, "A live fixture blocks the same team and time");
    }
    league.postponeFixture("F1");
    check(league.getFixture("F1").getStatus() == FixtureStatus::Postponed,
          "A postponed fixture reports its status");
    league.scheduleFixture("F2", "A", "C", "S1", "2026-12-08", "15:00");
    check(league.getFixtures().size() == 3, "A postponed fixture frees its team and time");
    try {
        league.recordResult("F1", MatchResult(1, 0));
        check(false, "Reject a result for a postponed fixture");
    } catch (const LeagueRuleError&) {
        check(!league.getFixture("F1").isCompleted(), "Reject a result for a postponed fixture");
    }
    league.recordResult("F2", MatchResult(0, 0));
    try {
        league.postponeFixture("F2");
        check(false, "Reject postponing a completed fixture");
    } catch (const LeagueRuleError&) {
        check(league.getFixture("F2").getStatus() == FixtureStatus::Completed,
              "Reject postponing a completed fixture");
    }
}

static void expectRuleError(const std::string& name, void (*action)()) {
    try {
        action();
        check(false, name);
    } catch (const LeagueRuleError&) {
        check(true, name);
    }
}

static void testSquads() {
    expectRuleError("Reject a player aged 15", [] {
        Player("P0", "Rajay Brown", 15, "M", 7, "Midfielder");
    });
    check(Player("P0", "Rajay Brown", 16, "M", 7, "Midfielder").getAge() == 16,
          "Accept a player aged exactly 16");
    expectRuleError("Reject jersey number 0", [] { Player("P0", "A B", 20, "M", 0, "Winger"); });
    expectRuleError("Reject jersey number 100", [] { Player("P0", "A B", 20, "M", 100, "Winger"); });
    check(Player("P0", "A B", 20, "M", 99, "Winger").getJerseyNumber() == 99,
          "Accept the highest jersey number");
    expectRuleError("Reject a blank player name", [] { Player("P0", "  ", 20, "M", 5, "Winger"); });
    expectRuleError("Reject a missing position", [] { Player("P0", "A B", 20, "M", 5, ""); });
    expectRuleError("Reject a coach without a qualification", [] {
        Coach("C0", "Alicia Brown", 35, "F", "", "Head Coach");
    });

    FootballLeague league;
    league.registerTeam(FootballTeam("A", "Harbour FC", "Kingston"));
    league.registerTeam(FootballTeam("B", "Metro FC", "St Andrew"));

    league.addPlayer("A", Player("P1", "Tajay Smith", 22, "M", 10, "Striker"));
    league.addPlayer("A", Player("P2", "Kevin Johnson", 25, "M", 4, "Defender"));
    check(league.getTeam("A").getSquadSize() == 2, "Squad size counts the players");

    try {
        league.addPlayer("A", Player("P3", "Kemar Reid", 24, "M", 10, "Winger"));
        check(false, "Reject a duplicate jersey number");
    } catch (const LeagueRuleError&) {
        check(league.getTeam("A").getSquadSize() == 2, "Reject a duplicate jersey number");
    }
    league.addPlayer("B", Player("P3", "Kemar Reid", 24, "M", 10, "Winger"));
    check(league.getTeam("B").getSquadSize() == 1, "The same jersey number is allowed on another team");
    try {
        league.addPlayer("B", Player("P1", "Tajay Smith", 22, "M", 11, "Striker"));
        check(false, "Reject a player already on another team");
    } catch (const LeagueRuleError&) {
        check(league.getTeam("B").getSquadSize() == 1, "Reject a player already on another team");
    }
    try {
        league.addPlayer("Z", Player("P9", "No Team", 22, "M", 1, "Keeper"));
        check(false, "Reject a player for an unknown team");
    } catch (const LeagueRuleError&) {
        check(true, "Reject a player for an unknown team");
    }

    league.updatePlayer("A", "P2", "Midfielder", 6, PlayingStatus::Injured);
    check(league.getTeam("A").getPlayer("P2").getPosition() == "Midfielder" &&
          league.getTeam("A").getPlayer("P2").getJerseyNumber() == 6 &&
          league.getTeam("A").getPlayer("P2").getPlayingStatus() == PlayingStatus::Injured,
          "Update a player's position, jersey and status");
    league.updatePlayer("A", "P2", "Midfielder", 6, PlayingStatus::Active);
    try {
        league.updatePlayer("A", "P2", "Midfielder", 10, PlayingStatus::Active);
        check(false, "Reject updating to a taken jersey number");
    } catch (const LeagueRuleError&) {
        check(league.getTeam("A").getPlayer("P2").getJerseyNumber() == 6,
              "Reject updating to a taken jersey number and keep the old one");
    }

    league.recordGoals("A", "P1", 2);
    league.recordGoals("A", "P1", 1);
    check(league.getTeam("A").getPlayer("P1").getGoals() == 3, "Goals add up");
    try {
        league.recordGoals("A", "P1", 0);
        check(false, "Reject adding zero goals");
    } catch (const LeagueRuleError&) {
        check(league.getTeam("A").getPlayer("P1").getGoals() == 3, "Reject adding zero goals");
    }

    league.removePlayer("A", "P2");
    check(league.getTeam("A").getSquadSize() == 1 && !league.getTeam("A").hasPlayer("P2"),
          "Remove a player");
    try {
        league.removePlayer("A", "P2");
        check(false, "Reject removing a player twice");
    } catch (const LeagueRuleError&) {
        check(true, "Reject removing a player twice");
    }

    league.addCoach("A", Coach("C1", "Alicia Brown", 35, "F", "UEFA A", "Head Coach"));
    check(league.getTeam("A").getCoaches().size() == 1 &&
          league.getTeam("A").getCoach("C1").getTeamId() == "A",
          "A coach added to a team is assigned to it");
    try {
        league.addCoach("B", Coach("C1", "Alicia Brown", 35, "F", "UEFA A", "Head Coach"));
        check(false, "Reject a coach already on another team");
    } catch (const LeagueRuleError&) {
        check(league.getTeam("B").getCoaches().empty(), "Reject a coach already on another team");
    }
    league.updateCoach("A", "C1", "UEFA Pro", "Technical Director");
    check(league.getTeam("A").getCoach("C1").getCoachingRole() == "Technical Director",
          "Update a coach");
    league.removeCoach("A", "C1");
    check(league.getTeam("A").getCoaches().empty(), "Remove a coach");

    check(league.searchTeams("harb").size() == 1 && league.searchTeams("KINGSTON").size() == 1 &&
          league.searchTeams("fc").size() == 2 && league.searchTeams("zzz").empty(),
          "Search teams by name or parish, ignoring case");

    std::ostringstream output;
    std::streambuf* original = std::cout.rdbuf(output.rdbuf());
    const Person& player = league.getTeam("A").getPlayers()[0];
    player.display();
    Spectator("S1", "Fan One", 30, "F").display();
    std::cout.rdbuf(original);
    check(output.str().find("Player") != std::string::npos &&
          output.str().find("Spectator") != std::string::npos,
          "Players and spectators print their own roles through Person");
}

static void testSales() {
    FootballLeague league;
    league.registerTeam(FootballTeam("A", "Harbour FC", "Kingston"));
    league.registerTeam(FootballTeam("B", "Metro FC", "St Andrew"));
    league.registerTeam(FootballTeam("C", "Cedar FC", "St Catherine"));
    league.registerTeam(FootballTeam("D", "Valley FC", "St Thomas"));
    for (const char* id : {"A", "B", "C", "D"}) {
        league.approveTeam(id);
    }
    league.registerStadium(Stadium("S1", "Tiny Park", "Kingston", 2, FieldCondition::Good, 115000));
    league.scheduleFixture("F1", "A", "B", "S1", "2026-12-01", "15:00");
    league.scheduleFixture("F2", "C", "D", "S1", "2026-12-08", "15:00");
    league.scheduleFixture("F3", "A", "C", "S1", "2026-12-15", "15:00");
    league.scheduleFixture("F4", "B", "D", "S1", "2026-12-22", "15:00");
    league.postponeFixture("F3");
    league.recordResult("F4", MatchResult(1, 1));

    StadiumShop shop;
    registerLeagueFixtures(league, shop);
    check(shop.hasFixture("F1") && shop.hasFixture("F2"),
          "Scheduled league fixtures are registered for ticket sales");
    check(!shop.hasFixture("F3") && !shop.hasFixture("F4"),
          "Postponed and completed fixtures are not registered for sales");
    registerLeagueFixtures(league, shop);
    check(shop.getTicketsSold("F1") == 0, "Registering the fixtures twice is harmless");

    Spectator fan("S1", "Fan One", 30, "F");
    SaleReceipt receipt = shop.sellTicket("T1", fan.getId(), "F1", "General", 115000);
    check(receipt.totalInCents == 115000 && receipt.taxInCents == 15000 &&
          receipt.subtotalInCents == 100000 && receipt.changeInCents == 0,
          "A ticket at the stadium's price includes 15 percent GCT");
    check(shop.sellTicket("T2", fan.getId(), "F1", "General", 120000).changeInCents == 5000,
          "A ticket sale returns the correct change");
    try {
        shop.sellTicket("T3", fan.getId(), "F1", "General", 115000);
        check(false, "Reject a ticket sale past the stadium capacity");
    } catch (const LeagueRuleError&) {
        check(shop.getTicketsSold("F1") == 2, "Reject a ticket sale past the stadium capacity");
    }
    check(shop.sellTicket("T4", fan.getId(), "F2", "General", 115000).totalInCents == 115000,
          "Another fixture at the same stadium has its own seats");
    try {
        shop.sellTicket("T5", fan.getId(), "F2", "General", 100);
        check(false, "Reject a ticket with insufficient payment");
    } catch (const LeagueRuleError&) {
        check(shop.getTicketsSold("F2") == 1, "Reject a ticket with insufficient payment");
    }
    try {
        shop.sellTicket("T6", fan.getId(), "F3", "General", 115000);
        check(false, "Reject a ticket for a postponed fixture");
    } catch (const SalesError&) {
        check(true, "Reject a ticket for a postponed fixture");
    }

    shop.addProduct(std::make_unique<Merchandise>("M1", "Jersey", 230000, 2, "Medium", "Harbour FC"));
    shop.addProduct(std::make_unique<FoodItem>("F1", "Water", 11500, 10, "Beverage"));
    check(shop.purchaseProduct("S1", "M1", 1, 250000).changeInCents == 20000,
          "A shop purchase returns the correct change");
    try {
        shop.purchaseProduct("S1", "M1", 2, 460000);
        check(false, "Reject buying more than the stock");
    } catch (const LeagueRuleError&) {
        check(shop.findProduct("M1").getStockQuantity() == 1, "Reject buying more than the stock");
    }
    try {
        shop.purchaseProduct("S1", "F1", 0, 0);
        check(false, "Reject a zero quantity purchase");
    } catch (const LeagueRuleError&) {
        check(shop.findProduct("F1").getStockQuantity() == 10, "Reject a zero quantity purchase");
    }

    std::vector<const Product*> items = {&shop.findProduct("M1"), &shop.findProduct("F1")};
    std::ostringstream output;
    for (const Product* item : items) {
        item->display(output);
    }
    check(output.str().find("Merchandise") != std::string::npos &&
          output.str().find("FoodItem") != std::string::npos,
          "Products print their own type through Product");
}

static void testReports() {
    FootballLeague league;
    // Boundary: an empty league still produces valid, empty reports.
    check(LeagueReports::standings(league).rows.empty() &&
          LeagueReports::fixtures(league).rows.empty() &&
          LeagueReports::officialAssignments(league).rows.empty() &&
          LeagueReports::teamStatus(league).rows.empty(), "Reports for an empty league have no rows");

    league.registerTeam(FootballTeam("A", "Harbour FC", "Kingston"));
    league.registerTeam(FootballTeam("B", "Metro, FC", "St Andrew"));
    league.approveTeam("A");
    league.approveTeam("B");
    league.registerStadium(Stadium("S1", "East Park", "Kingston", 100, FieldCondition::Good, 500));
    league.registerOfficial(MatchOfficial("R1", "Ref One", 32, "F"));
    league.registerOfficial(MatchOfficial("R2", "Ref Two", 40, "M"));
    league.scheduleFixture("F1", "A", "B", "S1", "2026-11-12", "15:00");
    league.assignOfficial("F1", "R1");
    league.recordResult("F1", MatchResult(3, 1));

    Report table = LeagueReports::standings(league);
    check(table.rows.size() == 2 && table.rows[0][1] == "Harbour FC" && table.rows[0][9] == "3" &&
          table.rows[1][9] == "0", "Standings report ranks the winner first with 3 points");
    Report fixtureReport = LeagueReports::fixtures(league);
    check(fixtureReport.rows.size() == 1 && fixtureReport.rows[0][7] == "3 - 1" &&
          fixtureReport.footer == "Completed: 1 of 1", "Fixtures report shows the score");
    Report officialReport = LeagueReports::officialAssignments(league);
    check(officialReport.rows.size() == 2 && officialReport.rows[0][2] == "1" &&
          officialReport.rows[1][2] == "0" && officialReport.rows[1][3] == "-",
          "Official report counts each official's fixtures");

    // The shared generator writes CSV, and a comma in a name must be quoted.
    std::filesystem::path file = std::filesystem::temp_directory_path() / "kpfl_report_test.csv";
    ReportGenerator::saveAsCsv(LeagueReports::teamStatus(league), file.string());
    std::ifstream in(file);
    std::string all((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    check(all.find("\"Metro, FC\"") != std::string::npos, "CSV report quotes a name with a comma");
    std::filesystem::remove(file);
}

static std::string standingsText(const FootballLeague& league) {
    std::string text;
    for (const Standing& row : league.calculateStandings()) {
        text += row.getTeamId() + ":" + std::to_string(row.getPoints()) + ":" +
                std::to_string(row.getGoalDifference()) + ";";
    }
    return text;
}

static void writeFile(const std::string& path, const std::string& text) {
    std::ofstream out(path);
    out << text;
}

// Save and reload through FileStore, then check the data and the fixture links survived.
static void testPersistence() {
    namespace fs = std::filesystem;
    fs::path original = fs::current_path();
    fs::path work = fs::temp_directory_path() / "kpfl_league_persistence_test";
    fs::remove_all(work);
    fs::create_directories(work);
    fs::current_path(work);

    FootballLeague league;
    league.registerTeam(FootballTeam("A", "Pipe | FC", "Kingston"));
    league.registerTeam(FootballTeam("B", "Metro FC", "St Andrew"));
    league.registerTeam(FootballTeam("C", "Cedar FC", "St Catherine"));
    league.registerTeam(FootballTeam("P", "Pending FC", "St Thomas"));
    league.approveTeam("A");
    league.approveTeam("B");
    league.approveTeam("C");
    league.registerStadium(Stadium("S1", "East Park", "Kingston", 100, FieldCondition::Good, 500));
    league.registerOfficial(MatchOfficial("R1", "Ref One", 32, "F"));
    league.registerOfficial(MatchOfficial("R2", "Ref Two", 40, "M"));
    league.scheduleFixture("F1", "A", "B", "S1", "2026-11-12", "15:00");
    league.assignOfficial("F1", "R1");
    league.assignOfficial("F1", "R2");
    league.scheduleFixture("F2", "B", "C", "S1", "2026-11-19", "15:00");
    league.recordResult("F1", MatchResult(2, 1));
    // A team suspended after scheduling must still load with its fixtures.
    league.suspendTeam("C");
    // Venue, availability and postponement details must survive the round trip too.
    league.reserveSeats("S1", 40);
    league.updateFieldCondition("S1", FieldCondition::Poor);
    league.setOfficialAvailability("R2", false);
    league.scheduleFixture("F3", "A", "B", "S1", "2026-12-01", "15:00");
    league.postponeFixture("F3");
    league.addPlayer("A", Player("P1", "Tajay | Smith", 22, "M", 10, "Striker",
                                 PlayingStatus::Injured, 4));
    league.addPlayer("A", Player("P2", "Kevin Johnson", 25, "M", 4, "Defender"));
    league.addPlayer("B", Player("P3", "Kemar Reid", 24, "M", 10, "Winger"));
    league.addCoach("A", Coach("C1", "Alicia Brown", 35, "F", "UEFA A", "Head Coach"));

    FileStore::save(league);
    FootballLeague reloaded;
    FileStore::load(reloaded);

    check(reloaded.getTeams().size() == 4 && reloaded.getTeam("A").getName() == "Pipe | FC",
          "Reload keeps teams, including a name with a pipe");
    check(reloaded.getTeam("C").getStatus() == TeamStatus::Suspended &&
          reloaded.getTeam("P").getStatus() == TeamStatus::Pending,
          "Reload keeps each team's status");
    check(reloaded.getTeam("A").getSquadSize() == 2 && reloaded.getTeam("B").getSquadSize() == 1 &&
          reloaded.getTeam("A").getPlayer("P1").getName() == "Tajay | Smith" &&
          reloaded.getTeam("A").getPlayer("P1").getGoals() == 4 &&
          reloaded.getTeam("A").getPlayer("P1").getPlayingStatus() == PlayingStatus::Injured,
          "Reload keeps each team's players with their details");
    check(reloaded.getTeam("A").getCoaches().size() == 1 &&
          reloaded.getTeam("A").getCoach("C1").getTeamId() == "A" &&
          reloaded.getTeam("A").getCoach("C1").getQualification() == "UEFA A",
          "Reload keeps a team's coach and assignment");
    check(reloaded.getStadium("S1").getCurrentOccupancy() == 40 &&
          reloaded.getStadium("S1").getFieldCondition() == FieldCondition::Poor &&
          reloaded.getStadium("S1").getTicketPriceCents() == 500,
          "Reload keeps stadium occupancy, condition and ticket price");
    check(reloaded.getOfficial("R1").isAvailable() && !reloaded.getOfficial("R2").isAvailable() &&
          reloaded.getOfficial("R2").getRank() == "Unranked",
          "Reload keeps official availability, role and rank");
    check(reloaded.getFixture("F3").getStatus() == FixtureStatus::Postponed,
          "Reload keeps a postponed fixture postponed");
    check(reloaded.getFixtures().size() == 3 && reloaded.getFixture("F2").getAwayTeamId() == "C",
          "Reload keeps fixtures, even for a suspended team");
    check(reloaded.getFixture("F1").getOfficials().size() == 2 &&
          reloaded.getFixture("F1").getOfficials()[1] == &reloaded.getOfficial("R2"),
          "Reloaded fixtures link to the reloaded officials");
    check(&reloaded.getFixture("F1").getHomeTeam() == &reloaded.getTeam("A"),
          "Reloaded fixtures link to the reloaded teams");
    check(reloaded.getFixture("F1").isCompleted() && !reloaded.getFixture("F2").isCompleted() &&
          reloaded.getFixture("F1").getResult().getHomeScore() == 2,
          "Reload keeps which fixtures have a result");
    check(standingsText(reloaded) == standingsText(league),
          "Reloading reproduces the same standings");

    // Data saved before the first match must load too: an empty results file is valid.
    FootballLeague fresh;
    check((FileStore::save(fresh), true), "Saving an empty league works");
    FootballLeague emptyReload;
    FileStore::load(emptyReload);
    check(emptyReload.getTeams().empty() && emptyReload.getFixtures().empty(),
          "Reloading an empty league gives an empty league");

    // Rewrite the saved league first, then damage single files to test the error paths.
    FileStore::save(league);
    std::string good = FileManager::loadLines(DataFiles::RESULTS)[0];

    writeFile(DataFiles::RESULTS, "F1|-3|0\n");
    FootballLeague damaged;
    try {
        FileStore::load(damaged);
        check(false, "Reject a negative score in the results file");
    } catch (const FileException&) {
        check(damaged.getTeams().empty() && damaged.getFixtures().empty(),
              "Reject a negative score in the results file and leave the league untouched");
    }

    writeFile(DataFiles::RESULTS, good + "\n");
    writeFile(DataFiles::FIXTURES, "F1|A|NOPE|S1|2026-11-12|15:00||0\n");
    try {
        FileStore::load(damaged);
        check(false, "Reject a fixture that names a missing team");
    } catch (const FileException&) {
        check(damaged.getFixtures().empty(), "Reject a fixture that names a missing team");
    }

    writeFile(DataFiles::FIXTURES, "F1|A|B\n");
    try {
        FileStore::load(damaged);
        check(false, "Reject a fixture record with missing fields");
    } catch (const DataFormatException&) {
        check(true, "Reject a fixture record with missing fields");
    }

    std::string goodPlayers = FileManager::loadLines(DataFiles::PLAYERS)[0];
    writeFile(DataFiles::PLAYERS, "NOPE|P9|Some One|20|M|5|Winger|0|0\n");
    try {
        FileStore::load(damaged);
        check(false, "Reject a player saved for a missing team");
    } catch (const FileException&) {
        check(damaged.getTeams().empty(), "Reject a player saved for a missing team");
    }
    writeFile(DataFiles::PLAYERS, "A|P9|Some One|12|M|5|Winger|0|0\n");
    try {
        FileStore::load(damaged);
        check(false, "Reject a saved player below the minimum age");
    } catch (const FileException&) {
        check(true, "Reject a saved player below the minimum age");
    }
    writeFile(DataFiles::PLAYERS, goodPlayers + "\n");

    writeFile(DataFiles::TEAMS, "A|One|Kingston|7\n");
    try {
        FileStore::load(damaged);
        check(false, "Reject an unknown team status code");
    } catch (const DataFormatException&) {
        check(true, "Reject an unknown team status code");
    }

    fs::current_path(original);
    fs::remove_all(work);
}

int main() {
    try {
        testScores();
        testLeague();
        testLinks();
        testPersistence();
        testReports();
        testVenueAndOfficials();
        testSquads();
        testSales();
    } catch (const LeagueRuleError& error) {
        check(false, std::string("Unexpected exception: ") + error.what());
    }
    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
