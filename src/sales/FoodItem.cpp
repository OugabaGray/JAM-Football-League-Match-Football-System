#include "sales/FoodItem.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

FoodItem::FoodItem(
    const std::string& productId,
    const std::string& name,
    std::int64_t priceInCents,
    int stockQuantity,
    const std::string& category)
    : Product(productId, name, priceInCents, stockQuantity)
{
    setCategory(category);
}

const std::string& FoodItem::getCategory() const
{
    return category;
}

void FoodItem::setCategory(const std::string& newCategory)
{
    if (newCategory != "Food" && newCategory != "Beverage")
    {
        throw std::invalid_argument(
            "Category must be Food or Beverage.");
    }

    category = newCategory;
}

std::string FoodItem::getType() const
{
    return "FoodItem";
}

void FoodItem::display(std::ostream& out) const
{
    std::ostringstream price;
    price << getPriceInCents() / 100 << '.'
        << std::setfill('0') << std::setw(2)
        << getPriceInCents() % 100;

    out << "Product ID: " << getProductId() << '\n'
        << "Name: " << getName() << '\n'
        << "Type: " << getType() << '\n'
        << "Category: " << category << '\n'
        << "Price (GCT included): J$" << price.str() << '\n'
        << "Stock available: " << getStockQuantity() << '\n';
}