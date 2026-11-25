# Individual Contribution Log

Role: Implementation (Integration) Lead
Name: Ougaba Gray (2405837)

Each module's classes are credited to the member who was in charge of
it, in that member's own contribution log and report section: team and
personnel (Tajay Smith), fixtures, stadiums and league operations
(Davian Dawkins), file management and reports (Jareel Edwards), and
ticketing and stadium shop (Sian Sudine).

## Work Completed
- Set up the shared repository, the module folders (include/ and src/
  per module), the module guide and the build and run instructions.
- Integrated the members' modules into one program that builds and
  runs as a single application.
- Wrote the main menu driver (src/main.cpp) that connects every
  module, including the ticketing and shop menu (option 20).
- Wrote FileStore, which saves and reloads the league through the file
  management module, and connected squads, stadiums, officials,
  fixtures and results to it.
- Linked the ticketing and shop module to the league's fixtures,
  stadium capacity and ticket price.
- Merged the file management additions (safe loading with backup
  recovery and money formatting), added the stadium summary report,
  and saved the demonstration data in the application's file formats.
- Maintained the combined test program (tests/LeagueTests.cpp) and
  added the safe-load, report and demonstration-data tests.
- Created the UML class diagram.
- Wrote the integration guide and test notes and kept the README up to
  date.
- Assembled the technical report: merged each member's section, wrote
  the design explanation, the conclusion and the references, and
  arranged real terminal screenshots of the integrated build.

## Validation
- The full test suite passes: 121 passed, 0 failed (normal, boundary
  and invalid inputs), built and run on a Mac and in a Linux container.
- Walked through a full match in the menu: register and approve two
  teams, schedule, assign an official, record 2-0, and check standings
  (3 points and 0 points).
- Verified save and reload reproduces the same standings.

## Supporting Evidence
- Git commits by Ougaba Gray in the repository history.
- docs/INTEGRATION_GUIDE.md, docs/TEST_NOTES.md and the UML file
  docs/Jamaican_Football_League_UML.drawio.
- Terminal screenshots of the test run and the menu walkthroughs are
  in the technical report (Figures 7 to 17).

## Remaining Work
- Assemble the final submission package.
- Build and rehearse the presentation slides and the demonstration.

## Assistance
AI assistance was used for coding guidance, troubleshooting and
documentation.
