#include "../../include/sales/Ticket.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

Ticket::Ticket(
    const std::string& ticketId,
    const std::string& spectatorId,
    const std::string& fixtureId,
    const std::string& category,
    std::int64_t priceInCents)
    : ticketId(ticketId),
    spectatorId(spectatorId),
    fixtureId(fixtureId),
    category(category),
    priceInCents(priceInCents)
{
    if (ticketId.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument("Ticket ID cannot be blank.");
    }

    if (spectatorId.find_first_not_of(" \t\r\n")
        == std::string::npos)
    {
        throw std::invalid_argument("Spectator ID cannot be blank.");
    }

    if (fixtureId.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument("Fixture ID cannot be blank.");
    }

    if (category.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument("Ticket category cannot be blank.");
    }

    if (priceInCents < 0)
    {
        throw std::invalid_argument("Ticket price cannot be negative.");
    }
}

const std::string& Ticket::getTicketId() const
{
    return ticketId;
}

const std::string& Ticket::getSpectatorId() const
{
    return spectatorId;
}

const std::string& Ticket::getFixtureId() const
{
    return fixtureId;
}

const std::string& Ticket::getCategory() const
{
    return category;
}

std::int64_t Ticket::getPriceInCents() const
{
    return priceInCents;
}

void Ticket::display(std::ostream& out) const
{
    std::ostringstream price;
    price << priceInCents / 100 << '.'
        << std::setfill('0') << std::setw(2)
        << priceInCents % 100;

    out << "--- MATCH TICKET ---\n"
        << "Ticket ID: " << ticketId << '\n'
        << "Spectator ID: " << spectatorId << '\n'
        << "Fixture ID: " << fixtureId << '\n'
        << "Category: " << category << '\n'
        << "Price (GCT included): J$" << price.str() << '\n';
}