#include "persistence/FileStore.h"
#include "FileManager.h"
#include "league/LeagueRuleError.h"
#include <utility>

void FileStore::save(const FootballLeague& league) {
    FileManager::saveCollection(DataFiles::TEAMS, league.teams, "teams");
    std::vector<std::string> playerLines;
    std::vector<std::string> coachLines;
    for (const std::unique_ptr<FootballTeam>& team : league.teams) {
        for (const Player& player : team->getPlayers()) {
            playerLines.push_back(Codec::escape(team->getRegistrationNumber()) + "|" +
                                  player.serialize());
        }
        for (const Coach& coach : team->getCoaches()) {
            coachLines.push_back(coach.serialize());
        }
    }
    FileManager::saveLines(DataFiles::PLAYERS, playerLines, "players");
    FileManager::saveLines(DataFiles::COACHES, coachLines, "coaches");
    FileManager::saveCollection(DataFiles::STADIUMS, league.stadiums, "stadiums");
    FileManager::saveCollection(DataFiles::OFFICIALS, league.officials, "officials");
    FileManager::saveCollection(DataFiles::FIXTURES, league.fixtures, "fixtures");

    // A result belongs to its fixture, so each line starts with the fixture ID.
    std::vector<std::string> resultLines;
    for (const Fixture& fixture : league.fixtures) {
        if (fixture.isCompleted()) {
            resultLines.push_back(Codec::escape(fixture.getId()) + "|" +
                                  fixture.getResult().serialize());
        }
    }
    FileManager::saveLines(DataFiles::RESULTS, resultLines, "results");
}

void FileStore::load(FootballLeague& league) {
    // Build everything in a scratch league first, then swap it in only if all of it loaded.
    FootballLeague loaded;
    try {
        for (const FootballTeam& team : FileManager::loadCollection<FootballTeam>(
                 DataFiles::TEAMS, FootballTeam::fromRecord)) {
            if (loaded.findTeam(team.getRegistrationNumber()) != -1) {
                throw DataFormatException("duplicate team " + team.getRegistrationNumber());
            }
            loaded.teams.push_back(std::make_unique<FootballTeam>(team));
        }
        for (const std::string& line : FileManager::loadLinesIfExists(DataFiles::PLAYERS)) {
            std::vector<std::string> fields = Codec::split(line);
            if (fields.size() < 2) {
                throw DataFormatException("a player record has no team");
            }
            std::string teamId = fields[0];
            fields.erase(fields.begin());
            loaded.addPlayer(teamId, Player::fromRecord(fields));
        }
        for (const Coach& coach : FileManager::loadCollection<Coach>(
                 DataFiles::COACHES, Coach::fromRecord)) {
            loaded.addCoach(coach.getTeamId(), coach);
        }
        for (const Stadium& stadium : FileManager::loadCollection<Stadium>(
                 DataFiles::STADIUMS, Stadium::fromRecord)) {
            loaded.registerStadium(stadium);
        }
        for (const MatchOfficial& official : FileManager::loadCollection<MatchOfficial>(
                 DataFiles::OFFICIALS, MatchOfficial::fromRecord)) {
            loaded.registerOfficial(official);
        }

        // Teams, stadiums and officials exist now, so each fixture can reconnect by ID.
        // Business rules are not re-checked: a team suspended since scheduling still has fixtures.
        for (const Fixture::Record& saved : FileManager::loadCollection<Fixture::Record>(
                 DataFiles::FIXTURES, Fixture::fromRecord)) {
            if (loaded.findFixture(saved.id) != -1) {
                throw DataFormatException("duplicate fixture " + saved.id);
            }
            Fixture fixture(saved.id, loaded.teams[loaded.requireTeam(saved.homeTeamId)].get(),
                            loaded.teams[loaded.requireTeam(saved.awayTeamId)].get(),
                            loaded.stadiums[loaded.requireStadium(saved.stadiumId)].get(),
                            saved.date, saved.time);
            for (const std::string& officialId : saved.officialIds) {
                fixture.addOfficial(loaded.officials[loaded.requireOfficial(officialId)].get());
            }
            // Postpone last: officials cannot be added to a postponed fixture.
            if (saved.postponed) {
                fixture.postpone();
            }
            loaded.fixtures.push_back(fixture);
        }

        for (const std::string& line : FileManager::loadLinesIfExists(DataFiles::RESULTS)) {
            std::vector<std::string> fields = Codec::split(line);
            Codec::requireFields(fields, 3, "Result");
            loaded.recordResult(fields[0],
                                MatchResult::fromRecord({fields[1], fields[2]}));
        }
    } catch (const LeagueRuleError& error) {
        // A broken reference or invalid value in a file is bad data, not a rule breach.
        throw DataFormatException(error.what());
    }

    // The moved objects keep their addresses, so the fixtures' pointers stay valid.
    league.teams = std::move(loaded.teams);
    league.stadiums = std::move(loaded.stadiums);
    league.officials = std::move(loaded.officials);
    league.fixtures = std::move(loaded.fixtures);
}
