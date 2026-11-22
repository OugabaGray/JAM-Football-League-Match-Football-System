#include "../../include/sales/Merchandise.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

Merchandise::Merchandise(
    const std::string& productId,
    const std::string& name,
    std::int64_t priceInCents,
    int stockQuantity,
    const std::string& size,
    const std::string& teamName)
    : Product(productId, name, priceInCents, stockQuantity)
{
    setSize(size);
    setTeamName(teamName);
}

const std::string& Merchandise::getSize() const
{
    return size;
}

const std::string& Merchandise::getTeamName() const
{
    return teamName;
}

void Merchandise::setSize(const std::string& newSize)
{
    if (newSize.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument(
            "Size cannot be blank. Use N/A if not applicable.");
    }

    size = newSize;
}

void Merchandise::setTeamName(const std::string& newTeamName)
{
    if (newTeamName.find_first_not_of(" \t\r\n")
        == std::string::npos)
    {
        throw std::invalid_argument(
            "Team name cannot be blank. Use N/A if not applicable.");
    }

    teamName = newTeamName;
}

std::string Merchandise::getType() const
{
    return "Merchandise";
}

void Merchandise::display(std::ostream& out) const
{
    // Format whole cents as dollars without rounding errors.
    std::ostringstream price;
    price << getPriceInCents() / 100 << '.'
        << std::setfill('0') << std::setw(2)
        << getPriceInCents() % 100;

    out << "Product ID: " << getProductId() << '\n'
        << "Name: " << getName() << '\n'
        << "Type: " << getType() << '\n'
        << "Team: " << teamName << '\n'
        << "Size: " << size << '\n'
        << "Price (GCT included): J$" << price.str() << '\n'
        << "Stock available: " << getStockQuantity() << '\n';
}