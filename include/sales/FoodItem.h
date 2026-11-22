#ifndef FOODITEM_H
#define FOODITEM_H

#include "Product.h"

class FoodItem : public Product
{
private:
    std::string category;

public:
    FoodItem(const std::string& productId,
        const std::string& name,
        std::int64_t priceInCents,
        int stockQuantity,
        const std::string& category);

    const std::string& getCategory() const;
    void setCategory(const std::string& newCategory);

    std::string getType() const override;
    void display(std::ostream& out) const override;
};

#endif