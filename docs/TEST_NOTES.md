# League test notes

The C++ test driver covers normal, boundary, and invalid inputs. Run it using the command in the main README.

| Input or action | Expected result |
| --- | --- |
| A home win, away win, or 0–0 draw | The points are 3/0, 0/3, or 1/1. |
| A negative score | A custom exception is caught and the fixture stays incomplete. |
| A commissioner and an official passed as `Person` references | Each object prints its own role. |
| A duplicate team number or a match involving a pending or suspended team | The league rejects the operation. |
| A team, stadium, or registered official booked at the same kickoff | The league rejects the second booking. |
| A fixture with no official, then with an assigned official | The inspection reports what is missing, then reports readiness. |
| One valid result followed by a second result for the same match | The first result completes the fixture and the second result is rejected. |
| Standings viewed twice after a 2–0 result | The winner stays on 3 points with one match played. |
| February 29 in a leap year at 23:59, then February 29 in a non-leap year | The valid kickoff is accepted and the invalid date is rejected. |

| A damaged file with no backup, then one with a good `.bak` copy | Safe load rejects the first and leaves the data untouched, and recovers the second from its backup (the damaged copy is kept as `.corrupt`). |
| A validator that rejects loaded data | Safe load treats it like a damaged file and leaves the data untouched. |
| Money of 150000, 5 and 0 cents | Shown as J$1,500.00, J$0.05 and J$0.00. |
| A stadium of 3000 seats at J$1,500.00 | The stadium report shows a full-house gate of J$4,500,000.00. |
| The demonstration data in `data/` | It loads six teams and twelve fixtures, and Kingston Harbour FC leads with 7 points and +4. |

The console walkthrough in the README also checks that these methods can be reached through the menu.
