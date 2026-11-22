#ifndef PRODUCT_H
#define PRODUCT_H

#include <cstdint>
#include <ostream>
#include <string>

class Product
{
private:
    std::string productId;
    std::string name;
    std::int64_t priceInCents; // Price includes 15% GCT
    int stockQuantity;

public:
    Product(const std::string& productId,
        const std::string& name,
        std::int64_t priceInCents,
        int stockQuantity);

    virtual ~Product() = default;

    const std::string& getProductId() const;
    const std::string& getName() const;
    std::int64_t getPriceInCents() const;
    int getStockQuantity() const;

    void setName(const std::string& newName);
    void setPriceInCents(std::int64_t newPrice);

    void restock(int quantity);
    void reduceStock(int quantity);

    virtual std::string getType() const = 0;
    virtual void display(std::ostream& out) const = 0;
};

#endif