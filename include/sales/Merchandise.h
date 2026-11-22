#ifndef MERCHANDISE_H
#define MERCHANDISE_H

#include "Product.h"

class Merchandise : public Product
{
private:
    std::string size;
    std::string teamName;

public:
    Merchandise(const std::string& productId,
        const std::string& name,
        std::int64_t priceInCents,
        int stockQuantity,
        const std::string& size,
        const std::string& teamName);

    const std::string& getSize() const;
    const std::string& getTeamName() const;

    void setSize(const std::string& newSize);
    void setTeamName(const std::string& newTeamName);

    std::string getType() const override;
    void display(std::ostream& out) const override;
};

#endif