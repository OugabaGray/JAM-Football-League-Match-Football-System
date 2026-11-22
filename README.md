# Jamaican Football League Match and Sales System

This is the C++ group project for CIT 2004. It manages team registration and approval, squads, fixture scheduling, official assignments, match scores, the league table, ticket sales, the stadium shop, saving and loading, and reports.

The project combines the league, fixtures and stadiums, team and personnel, ticketing and shop, and file-management sections. See the [integration notes](docs/INTEGRATION_GUIDE.md) for how the parts fit together.

## Build and run

Open a terminal in this folder. Use a C++17 compiler such as `g++`.

```sh
g++ -std=c++17 -Iinclude -Iinclude/persistence src/main.cpp src/*/*.cpp -o jam_football_league
./jam_football_league
```

In PowerShell with MinGW-w64, list the source files explicitly and run `.\jam_football_league.exe`:

```powershell
g++ -std=c++17 -Iinclude -Iinclude/persistence (Get-ChildItem src -Recurse -Filter *.cpp).FullName -o jam_football_league.exe
```

## Try one match

1. Add two teams with option 1. Use registration numbers `A` and `B`. Approve both with option 5.
2. Add a playable stadium `S1` with option 7 and a match official `R1` with option 8.
3. Schedule `F1` with option 9. Use `A` as home, `B` as away, and `S1` as the stadium. Enter the date as `YYYY-MM-DD` and the kickoff as `HH:MM`.
4. Assign `R1` to `F1` with option 10, then inspect the fixture with option 11.
5. Record a 2–0 score with option 12 and open the standings with option 14. The home team should have 3 points and the away team 0.

Options 15 and 16 save and reload the league using the file-management module (`include/persistence`, `src/persistence`, guide in `docs/FILE_MODULE_GUIDE.md`). Files go in `data/`, relative to where you run the program. Option 17 shows one of four reports and saves it as text and CSV in `data/reports/`. Options 18 and 19 postpone a fixture and set an official's availability. Option 20 opens the ticketing and shop menu: the league's scheduled fixtures are registered for ticket sales with their stadium's capacity and price, and the menu handles spectators, products, purchases, receipts, reports and the shop's own save and load (see `docs/TicketingShop.md`). Options 21 to 29 manage squads: add, update and remove players and coaches, show a squad, search teams and record player goals.

## Run the checks

```sh
g++ -std=c++17 -Iinclude -Iinclude/persistence tests/LeagueTests.cpp src/*/*.cpp -o league_tests
./league_tests
```

The sales module also has its own demonstration tests:

```sh
g++ -std=c++17 -Iinclude tests/SalesDemo.cpp src/sales/Product.cpp src/sales/Merchandise.cpp src/sales/FoodItem.cpp src/sales/Ticket.cpp src/sales/StadiumShop.cpp -o sales_demo
./sales_demo
```

In PowerShell with MinGW-w64, run `g++ -std=c++17 -Iinclude -Iinclude/persistence tests/LeagueTests.cpp (Get-ChildItem src -Recurse -Filter *.cpp | Where-Object Name -ne main.cpp).FullName -o league_tests.exe`, then `.\league_tests.exe`. See the [test notes](docs/TEST_NOTES.md) for the expected results.

## Course references

The design follows the CIT 2004 class, information hiding, inheritance, exception handling, and polymorphism exercises. The [project brief](Jamaican_Football_League_6_Week_OOP_Group_Project.pdf) covers the full group submission. AI assistance was used for coding guidance, troubleshooting and documentation.
