# Jamaican Football League Match and Sales System

This is our OOP group project. We're building a program to keep track of teams, fixtures, and ticket sales for the Jamaican Football League. The [assignment brief](Jamaican_Football_League_6_Week_OOP_Group_Project.pdf) has the full requirements.

## Where things go

- `include/` is for headers and `src/` is for `.cpp` files. The program starts in `src/main.cpp`.
- `tests/` is for tests, `data/` is for sample data, and `docs/` is for project notes.

If you're working on fixtures, put your header in `include/fixtures/` and your `.cpp` file in `src/fixtures/`. The [module guide](docs/README.md) lists the other folders.

## How to run it

Open a terminal in the project folder. You'll need a C++17 compiler. These commands use `g++`.

**macOS or Linux**

```sh
g++ -std=c++17 -Iinclude src/main.cpp -o jam_football_league
./jam_football_league
```

**Windows (PowerShell with MinGW-w64)**

```powershell
g++ -std=c++17 -Iinclude src/main.cpp -o jam_football_league.exe
.\jam_football_league.exe
```

As we add classes, list their `.cpp` files after `src/main.cpp` in the compile command.
