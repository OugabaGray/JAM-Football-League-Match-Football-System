#include "sales/Product.h"

#include <limits>
#include <stdexcept>

Product::Product(const std::string& productId,
    const std::string& name,
    std::int64_t priceInCents,
    int stockQuantity)
    : productId(productId),
    name(name),
    priceInCents(priceInCents),
    stockQuantity(stockQuantity)
{
    if (productId.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument("Product ID cannot be blank.");
    }

    setName(name);
    setPriceInCents(priceInCents);

    if (stockQuantity < 0)
    {
        throw std::invalid_argument("Stock cannot be negative.");
    }
}

const std::string& Product::getProductId() const
{
    return productId;
}

const std::string& Product::getName() const
{
    return name;
}

std::int64_t Product::getPriceInCents() const
{
    return priceInCents;
}

int Product::getStockQuantity() const
{
    return stockQuantity;
}

void Product::setName(const std::string& newName)
{
    if (newName.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument("Product name cannot be blank.");
    }

    name = newName;
}

void Product::setPriceInCents(std::int64_t newPrice)
{
    if (newPrice < 0)
    {
        throw std::invalid_argument("Price cannot be negative.");
    }

    priceInCents = newPrice;
}

void Product::restock(int quantity)
{
    if (quantity <= 0)
    {
        throw std::invalid_argument(
            "Restock quantity must be greater than zero.");
    }

    if (quantity > std::numeric_limits<int>::max() - stockQuantity)
    {
        throw std::overflow_error("Stock quantity is too large.");
    }

    stockQuantity += quantity;
}

void Product::reduceStock(int quantity)
{
    if (quantity <= 0)
    {
        throw std::invalid_argument(
            "Purchase quantity must be greater than zero.");
    }

    if (quantity > stockQuantity)
    {
        throw std::runtime_error("Insufficient stock.");
    }

    stockQuantity -= quantity;
}