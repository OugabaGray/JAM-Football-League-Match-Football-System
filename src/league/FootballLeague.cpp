#include "league/FootballLeague.h"
#include "league/LeagueRuleError.h"
#include <algorithm>
#include <cctype>

int FootballLeague::findTeam(const std::string& teamId) const {
    for (std::size_t i = 0; i < teams.size(); i++) {
        if (teams[i]->getRegistrationNumber() == teamId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int FootballLeague::findStadium(const std::string& stadiumId) const {
    for (std::size_t i = 0; i < stadiums.size(); i++) {
        if (stadiums[i]->getId() == stadiumId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int FootballLeague::findFixture(const std::string& fixtureId) const {
    for (std::size_t i = 0; i < fixtures.size(); i++) {
        if (fixtures[i].getId() == fixtureId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int FootballLeague::findOfficial(const std::string& officialId) const {
    for (std::size_t i = 0; i < officials.size(); i++) {
        if (officials[i]->getId() == officialId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int FootballLeague::requireTeam(const std::string& teamId) const {
    int index = findTeam(teamId);
    if (index == -1) {
        throw LeagueRuleError("Team not found: " + teamId);
    }
    return index;
}

int FootballLeague::requireStadium(const std::string& stadiumId) const {
    int index = findStadium(stadiumId);
    if (index == -1) {
        throw LeagueRuleError("Stadium not found: " + stadiumId);
    }
    return index;
}

int FootballLeague::requireOfficial(const std::string& officialId) const {
    int index = findOfficial(officialId);
    if (index == -1) {
        throw LeagueRuleError("Match official not found: " + officialId);
    }
    return index;
}

int FootballLeague::requireFixture(const std::string& fixtureId) const {
    int index = findFixture(fixtureId);
    if (index == -1) {
        throw LeagueRuleError("Fixture not found: " + fixtureId);
    }
    return index;
}

// Registration adds a pending team. Approval is a separate commissioner action.
void FootballLeague::registerTeam(const FootballTeam& team) {
    if (findTeam(team.getRegistrationNumber()) != -1) {
        throw LeagueRuleError("A team already uses that registration number.");
    }
    if (team.getStatus() != TeamStatus::Pending) {
        throw LeagueRuleError("New teams must start pending approval.");
    }
    // The league keeps its own copy on the heap so the address stays fixed.
    teams.push_back(std::make_unique<FootballTeam>(team));
}

void FootballLeague::updateTeam(const std::string& teamId, const std::string& name,
                                const std::string& parish) {
    // Edit the stored team. Fixtures point at it, so they see the new details too.
    teams[requireTeam(teamId)]->updateDetails(name, parish);
}

void FootballLeague::approveTeam(const std::string& teamId) {
    teams[requireTeam(teamId)]->approveRegistration();
}

void FootballLeague::suspendTeam(const std::string& teamId) {
    teams[requireTeam(teamId)]->suspendRegistration();
}

const FootballTeam& FootballLeague::getTeam(const std::string& teamId) const {
    return *teams[requireTeam(teamId)];
}

std::vector<const FootballTeam*> FootballLeague::getTeams() const {
    std::vector<const FootballTeam*> view;
    for (const std::unique_ptr<FootballTeam>& team : teams) {
        view.push_back(team.get());
    }
    return view;
}

static std::string lowerCase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::vector<const FootballTeam*> FootballLeague::searchTeams(const std::string& text) const {
    std::string needle = lowerCase(text);
    std::vector<const FootballTeam*> matches;
    for (const std::unique_ptr<FootballTeam>& team : teams) {
        if (lowerCase(team->getName()).find(needle) != std::string::npos ||
            lowerCase(team->getParish()).find(needle) != std::string::npos ||
            lowerCase(team->getRegistrationNumber()).find(needle) != std::string::npos) {
            matches.push_back(team.get());
        }
    }
    return matches;
}

void FootballLeague::addPlayer(const std::string& teamId, const Player& player) {
    FootballTeam& team = *teams[requireTeam(teamId)];
    for (const std::unique_ptr<FootballTeam>& other : teams) {
        if (other->hasPlayer(player.getId())) {
            throw LeagueRuleError("That player ID is already registered with a team.");
        }
    }
    team.addPlayer(player);
}

void FootballLeague::updatePlayer(const std::string& teamId, const std::string& playerId,
                                  const std::string& position, int jerseyNumber,
                                  PlayingStatus status) {
    teams[requireTeam(teamId)]->updatePlayer(playerId, position, jerseyNumber, status);
}

void FootballLeague::recordGoals(const std::string& teamId, const std::string& playerId,
                                 int goals) {
    teams[requireTeam(teamId)]->recordGoals(playerId, goals);
}

void FootballLeague::removePlayer(const std::string& teamId, const std::string& playerId) {
    teams[requireTeam(teamId)]->removePlayer(playerId);
}

void FootballLeague::addCoach(const std::string& teamId, const Coach& coach) {
    FootballTeam& team = *teams[requireTeam(teamId)];
    for (const std::unique_ptr<FootballTeam>& other : teams) {
        if (other->hasCoach(coach.getId())) {
            throw LeagueRuleError("That coach ID is already registered with a team.");
        }
    }
    team.addCoach(coach);
}

void FootballLeague::updateCoach(const std::string& teamId, const std::string& coachId,
                                 const std::string& qualification,
                                 const std::string& coachingRole) {
    teams[requireTeam(teamId)]->updateCoach(coachId, qualification, coachingRole);
}

void FootballLeague::removeCoach(const std::string& teamId, const std::string& coachId) {
    teams[requireTeam(teamId)]->removeCoach(coachId);
}

void FootballLeague::registerStadium(const Stadium& stadium) {
    if (findStadium(stadium.getId()) != -1) {
        throw LeagueRuleError("A stadium already uses that ID.");
    }
    stadiums.push_back(std::make_unique<Stadium>(stadium));
}

const Stadium& FootballLeague::getStadium(const std::string& stadiumId) const {
    return *stadiums[requireStadium(stadiumId)];
}

std::vector<const Stadium*> FootballLeague::getStadiums() const {
    std::vector<const Stadium*> view;
    for (const std::unique_ptr<Stadium>& stadium : stadiums) {
        view.push_back(stadium.get());
    }
    return view;
}

std::vector<const MatchOfficial*> FootballLeague::getOfficials() const {
    std::vector<const MatchOfficial*> view;
    for (const std::unique_ptr<MatchOfficial>& official : officials) {
        view.push_back(official.get());
    }
    return view;
}

void FootballLeague::registerOfficial(const MatchOfficial& official) {
    if (findOfficial(official.getId()) != -1) {
        throw LeagueRuleError("A match official already uses that ID.");
    }
    officials.push_back(std::make_unique<MatchOfficial>(official));
}

const MatchOfficial& FootballLeague::getOfficial(const std::string& officialId) const {
    return *officials[requireOfficial(officialId)];
}

void FootballLeague::scheduleFixture(const std::string& fixtureId,
                                     const std::string& homeTeamId,
                                     const std::string& awayTeamId,
                                     const std::string& stadiumId,
                                     const std::string& date, const std::string& time) {
    if (findFixture(fixtureId) != -1) {
        throw LeagueRuleError("A fixture already uses that ID.");
    }

    // Resolve the IDs to the league's stored objects, then build the fixture around them.
    // The Fixture constructor checks the ID, the two-team rule and the date and time.
    const FootballTeam* home = teams[requireTeam(homeTeamId)].get();
    const FootballTeam* away = teams[requireTeam(awayTeamId)].get();
    const Stadium* stadium = stadiums[requireStadium(stadiumId)].get();
    Fixture fixture(fixtureId, home, away, stadium, date, time);

    if (home->getStatus() != TeamStatus::Registered ||
        away->getStatus() != TeamStatus::Registered) {
        throw LeagueRuleError("Both teams must be registered and active.");
    }
    if (!stadium->isPlayable()) {
        throw LeagueRuleError("The selected stadium is not playable.");
    }

    // A clash means the same date and kickoff time.
    for (const Fixture& other : fixtures) {
        if (other.isPostponed()) {
            continue;
        }
        if (other.getDate() == fixture.getDate() && other.getTime() == fixture.getTime()) {
            if (other.involvesTeam(fixture.getHomeTeamId()) ||
                other.involvesTeam(fixture.getAwayTeamId())) {
                throw LeagueRuleError("A team already has a match at that time.");
            }
            if (other.getStadiumId() == fixture.getStadiumId()) {
                throw LeagueRuleError("That stadium is already booked at that time.");
            }
        }
    }
    // Add the fixture only after all checks pass.
    fixtures.push_back(fixture);
}

void FootballLeague::assignOfficial(const std::string& fixtureId, const std::string& officialId) {
    const MatchOfficial* official = officials[requireOfficial(officialId)].get();
    if (!official->isAvailable()) {
        throw LeagueRuleError("That match official is marked as unavailable.");
    }

    // Use a reference so addOfficial updates the fixture in the league's list.
    Fixture& fixture = fixtures[requireFixture(fixtureId)];
    for (const Fixture& other : fixtures) {
        if (!other.isPostponed() && other.getId() != fixtureId && other.getDate() == fixture.getDate() &&
            other.getTime() == fixture.getTime() && other.hasOfficial(officialId)) {
            throw LeagueRuleError("That official already has a match at that time.");
        }
    }
    fixture.addOfficial(official);
}

void FootballLeague::setOfficialAvailability(const std::string& officialId, bool available) {
    officials[requireOfficial(officialId)]->setAvailability(available);
}

void FootballLeague::postponeFixture(const std::string& fixtureId) {
    fixtures[requireFixture(fixtureId)].postpone();
}

void FootballLeague::updateFieldCondition(const std::string& stadiumId, FieldCondition condition) {
    stadiums[requireStadium(stadiumId)]->updateFieldCondition(condition);
}

void FootballLeague::reserveSeats(const std::string& stadiumId, int count) {
    stadiums[requireStadium(stadiumId)]->bookSeats(count);
}

void FootballLeague::recordResult(const std::string& fixtureId, const MatchResult& result) {
    // Fixture owns the score and switches to completed when it accepts the result.
    fixtures[requireFixture(fixtureId)].recordResult(result);
}

const Fixture& FootballLeague::getFixture(const std::string& fixtureId) const {
    return fixtures[requireFixture(fixtureId)];
}

const std::vector<Fixture>& FootballLeague::getFixtures() const {
    return fixtures;
}

// Points decide the rank first. The remaining comparisons are proposed tie-breaks.
static bool ranksAbove(const Standing& first, const Standing& second) {
    if (first.getPoints() != second.getPoints()) {
        return first.getPoints() > second.getPoints();
    }
    if (first.getGoalDifference() != second.getGoalDifference()) {
        return first.getGoalDifference() > second.getGoalDifference();
    }
    if (first.getGoalsFor() != second.getGoalsFor()) {
        return first.getGoalsFor() > second.getGoalsFor();
    }
    if (first.getTeamName() != second.getTeamName()) {
        return first.getTeamName() < second.getTeamName();
    }
    return first.getTeamId() < second.getTeamId();
}

std::vector<Standing> FootballLeague::calculateStandings() const {
    std::vector<Standing> table;
    // Approved teams get a row even before playing. Suspended teams keep their history.
    for (const std::unique_ptr<FootballTeam>& team : teams) {
        if (team->getStatus() != TeamStatus::Pending) {
            table.push_back(Standing(team->getRegistrationNumber(), team->getName()));
        }
    }

    // Start with a fresh table so viewing it twice cannot count a match twice.
    for (const Fixture& fixture : fixtures) {
        if (!fixture.isCompleted()) {
            continue;
        }
        const MatchResult& result = fixture.getResult();
        // Each completed fixture updates both teams. The away team reads the score in reverse.
        for (Standing& row : table) {
            if (row.getTeamId() == fixture.getHomeTeamId()) {
                row.addMatch(result.getHomeScore(), result.getAwayScore(), result.getHomePoints());
            } else if (row.getTeamId() == fixture.getAwayTeamId()) {
                row.addMatch(result.getAwayScore(), result.getHomeScore(), result.getAwayPoints());
            }
        }
    }

    // The named comparison above tells sort which of two rows comes first.
    std::sort(table.begin(), table.end(), ranksAbove);
    return table;
}
