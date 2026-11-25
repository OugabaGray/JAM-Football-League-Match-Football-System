# Individual Contribution Log

Role: Project Leader and Integration
Name: Ougaba Gray (2405837)

## Work Completed
- Created the shared repository and the module folder structure
  (include/ and src/ per module) and the module guide.
- Wrote the build and run instructions in the README.
- Implemented the league module: FootballLeague, FootballTeam,
  Standing and the LeagueRuleError custom exception.
- Implemented the fixtures module: Fixture, Stadium and MatchResult.
- Implemented the personnel classes: Person, Player, Coach,
  MatchOfficial and LeagueCommissioner.
- Implemented league rules: only registered teams may play, no
  team, stadium or official double-booked at the same kickoff,
  non-negative scores, one result per fixture, and 3/1/0 points
  with calculated standings and tie-breaks.
- Integrated the file-management module (Persistable, FileStore,
  FileManager, ReportGenerator) so teams, stadiums, officials,
  fixtures, results and squads save and reload.
- Built the main menu driver (src/main.cpp) and the league test
  driver (tests/LeagueTests.cpp).
- Integrated the ticketing and shop module with the league, so
  ticket sales use the league's fixtures and stadium capacity and
  price.
- Created the updated UML class diagram.
- Wrote the integration guide and test notes.
- Merged the file management additions (safe loading with backup
  recovery and money formatting), added the stadium summary report,
  and saved the demonstration data in the application's file formats.
- Merged each member's section into the technical report and wrote
  its conclusion and references.

## Validation
- The full test suite passes: 121 passed, 0 failed (normal, boundary
  and invalid inputs), built and run on a Mac and in a Linux container.
- Walked through a full match in the menu: register and approve
  two teams, schedule, assign an official, record 2-0, and check
  standings (3 points and 0 points).
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
