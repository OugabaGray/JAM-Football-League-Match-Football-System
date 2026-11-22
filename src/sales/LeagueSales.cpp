#include "sales/LeagueSales.h"

void registerLeagueFixtures(const FootballLeague& league, StadiumShop& shop)
{
    for (const Fixture& fixture : league.getFixtures())
    {
        if (fixture.getStatus() != FixtureStatus::Scheduled
            || shop.hasFixture(fixture.getId()))
        {
            continue;
        }

        const Stadium& stadium = fixture.getStadium();

        shop.registerFixture(
            fixture.getId(),
            stadium.getSeatingCapacity(),
            stadium.getTicketPriceCents());
    }
}
