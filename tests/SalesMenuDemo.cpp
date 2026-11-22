#include "sales/SalesMenu.h"
#include "sales/Merchandise.h"
#include "sales/FoodItem.h"

#include <iostream>
#include <memory>

int main()
{
    try
    {
        StadiumShop shop;

        // Sample products: prices include GCT and use cents.
        shop.addProduct(std::make_unique<Merchandise>(
            "M001", "Team Jersey", 230000, 5,
            "Medium", "Kingston FC"));

        shop.addProduct(std::make_unique<FoodItem>(
            "F001", "Bottled Water", 11500, 10,
            "Beverage"));

        // Sample fixture for testing ticket purchases.
        shop.registerFixture("FIX001", 100, 115000);

        runSalesMenu(shop);

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}