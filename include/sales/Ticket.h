#ifndef TICKET_H
#define TICKET_H

#include <cstdint>
#include <ostream>
#include <string>

class Ticket
{
private:
    std::string ticketId;
    std::string spectatorId;
    std::string fixtureId;
    std::string category;
    std::int64_t priceInCents;

public:
    Ticket(const std::string& ticketId,
        const std::string& spectatorId,
        const std::string& fixtureId,
        const std::string& category,
        std::int64_t priceInCents);

    const std::string& getTicketId() const;
    const std::string& getSpectatorId() const;
    const std::string& getFixtureId() const;
    const std::string& getCategory() const;
    std::int64_t getPriceInCents() const;

    void display(std::ostream& out) const;
};

#endif