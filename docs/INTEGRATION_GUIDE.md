# Integration notes

## Class responsibilities

| Class | What it does |
| --- | --- |
| `FootballLeague` | Keeps the league's collections and checks rules that involve more than one object. |
| `LeagueCommissioner` | Is a `Person` who approves teams, assigns officials, and inspects match preparations. |
| `MatchResult` | Holds two private scores, rejects negative scores, and returns the points for each side. |
| `Standing` | Holds one team's totals while the league works out the table from completed fixtures. |
| `LeagueRuleError` | Gives the driver a custom exception to catch when a rule is broken. |

A fixture links to exactly two distinct teams and one stadium. Teams and stadiums can be linked to several fixtures over the season. Officials also exist independently of fixtures and can be assigned to several at different times. These are aggregations: the fixture holds non-owning pointers (`const FootballTeam*`, `const Stadium*`, `const MatchOfficial*`) and the `FootballLeague` owns the objects through `std::unique_ptr`, so their addresses stay fixed when a list grows. A fixture owns zero or one result, which is composition. The driver calls `display()` through a `Person&` to show that a commissioner and a match official respond differently to the same method call.

## Agree on the shared class interfaces

Each module has headers under `include/<module>/` and implementation files under `src/<module>/`. `personnel` holds `Person`, `Player`, `Coach`, `Spectator`, `MatchOfficial` and `LeagueCommissioner`. `fixtures` holds `Fixture`, `Stadium` and `MatchResult`. `league` holds `FootballLeague`, `FootballTeam` and `Standing`. `persistence` and `reports` hold the file and report classes. The Section C features are merged in (see below). Agree on any signature changes first, then update the calls in the league and driver together.

| Shared class | What this section needs |
| --- | --- |
| `personnel/Person.h` | An abstract base with ID, name, age and gender (all strings except age), and `virtual void display() const = 0`. |
| `league/FootballTeam.h` | Registration number, name, parish, and pending/registered/suspended status. The league needs to read these values, update team details, approve registration, and suspend a team. |
| `fixtures/Stadium.h` | Stadium ID, name, location, capacity, `FieldCondition`, ticket price in cents and occupancy. `bookSeats()` refuses sales over capacity. A stadium is playable unless its condition is `Unplayable`. |
| `personnel/MatchOfficial.h` | A `Person` subclass with role, rank and availability. The league refuses unavailable officials and checks current bookings when it assigns one. |
| `fixtures/Fixture.h` | Fixture ID, both team IDs, stadium ID, date, kickoff, official IDs, and a result once recorded. The fixture must be able to accept an official or record a score through a method. |

The league keeps one list per shared class and owns every object in it. Make updates through the league. IDs are still the lookup keys and the saved form: when save/load is added, write the IDs and data fields to sequential text files, load teams, stadiums and officials first, then rebuild each fixture with `scheduleFixture`-style lookups so its pointers reconnect to the loaded objects. Reloading results should reproduce the same standings. A fixture's pointers must never outlive the league, and `FootballLeague` is non-copyable for that reason.

## Decisions to review together

- A kickoff conflict uses the same date and minute. The brief gives no match length.
- One assigned official is enough for the demo's readiness check. Discuss actual staffing and availability with the fixtures teammate.
- Teams are ranked by points, then goal difference, goals scored, team name, and registration number. The 3/1/0 points rule comes from the brief. The tie-break order is proposed for the group to confirm.
- Approvals, fixtures and results live in memory and are saved and reloaded through the persistence module. Team and personnel, the sales and product hierarchy, and reports are now integrated with the league.

## Persistence hookup

The league classes use the file-management module's `Persistable` and `Codec`. Saved formats:

| File | Line format |
| --- | --- |
| `teams.txt` | `registrationNumber\|name\|parish\|status` (0 pending, 1 registered, 2 suspended) |
| `stadiums.txt` | `id\|name\|location\|capacity\|playable` |
| `officials.txt` | `id\|name\|age\|gender` |
| `fixtures.txt` | `id\|homeTeamId\|awayTeamId\|stadiumId\|date\|time\|officialIds` |
| `results.txt` | `fixtureId\|homeScore\|awayScore`, one line per completed fixture |

`FileStore::load` reads teams, stadiums and officials first, then rebuilds each fixture and reattaches its pointers by ID. It builds everything in a scratch league and swaps it in only if all files load, so a damaged file leaves the current data untouched. It does not re-check business rules on load. Standings are never saved, because they are calculated from the results. `FileStore` is a friend of `FootballLeague`. `FileManager::safeLoad` (load into temporary storage, validate, restore from the `.bak` copy and keep the damaged file as `.corrupt`) is available for any file, and the demonstration data in `data/` is saved in the formats above. Players, coaches, products and tickets still need their own `serialize()` and `fromRecord()` from their owners.

## Section C merge

Section C's design was merged into the shared classes rather than kept as a second league. What came from it: stadium field condition, ticket price and occupancy with `bookSeats`, the official's role, rank and availability, and fixture postponement. What changed: field condition is an enum, money is in cents, postponed fixtures free their team, stadium and officials, and everything saves through `Persistable`. Standings stay calculated from results, and a fixture rejects a second result. `FootballLeague` has `reserveSeats`, `setOfficialAvailability`, `postponeFixture` and `updateFieldCondition` for the menu and for the ticketing teammate.

## Team and personnel

`Player`, `Coach` and `Spectator` derive from `Person` and `Persistable`. A player must be at least 16 (`Player::MINIMUM_AGE`) and has a jersey number from 1 to 99, a position, goals and a playing status. A coach has a qualification, a coaching role and the team it is assigned to. A `FootballTeam` owns its players and coaches (composition): it refuses a duplicate player or coach and a duplicate jersey number, and `FootballLeague` also refuses a player or coach ID already on another team. Squad size is the number of players. Squads save to `players.txt` (each line starts with the team's registration number) and `coaches.txt` (each line holds the team ID).
