#include "personnel/LeagueCommissioner.h"
#include "league/FootballLeague.h"
#include <iostream>

LeagueCommissioner::LeagueCommissioner(const std::string& id, const std::string& name,
                                       int age, const std::string& gender)
    // Initialise the inherited identity through the Person constructor.
    : Person(id, name, age, gender) {}

void LeagueCommissioner::display() const {
    std::cout << getName() << " (League Commissioner " << getId() << ")\n";
}

void LeagueCommissioner::approveRegistration(FootballLeague& league,
                                             const std::string& teamId) const {
    // The commissioner requests approval. The league changes its stored team.
    league.approveTeam(teamId);
}

void LeagueCommissioner::suspendRegistration(FootballLeague& league,
                                             const std::string& teamId) const {
    league.suspendTeam(teamId);
}

void LeagueCommissioner::assignMatchOfficial(FootballLeague& league,
                                             const std::string& fixtureId,
                                             const std::string& officialId) const {
    league.assignOfficial(fixtureId, officialId);
}

std::string LeagueCommissioner::inspectMatchPreparations(const FootballLeague& league,
                                                         const std::string& fixtureId) const {
    // Read the current team and venue details, since they may differ from kickoff planning.
    const Fixture& fixture = league.getFixture(fixtureId);
    if (fixture.isCompleted()) {
        return "This fixture is already complete.";
    }
    if (fixture.isPostponed()) {
        return "This fixture is postponed.";
    }
    if (league.getTeam(fixture.getHomeTeamId()).getStatus() != TeamStatus::Registered ||
        league.getTeam(fixture.getAwayTeamId()).getStatus() != TeamStatus::Registered) {
        return "Both teams must remain registered.";
    }
    if (!league.getStadium(fixture.getStadiumId()).isPlayable()) {
        return "The stadium is not ready for play.";
    }
    if (!fixture.hasOfficials()) {
        return "Assign at least one match official before kickoff.";
    }
    for (const MatchOfficial* official : fixture.getOfficials()) {
        if (!official->isAvailable()) {
            return "An assigned match official is no longer available.";
        }
    }
    return "Teams, stadium and official assignment are ready.";
}
