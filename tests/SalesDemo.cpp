#include "sales/StadiumShop.h"
#include "sales/Merchandise.h"
#include "sales/FoodItem.h"

#include <iostream>
#include <memory>
#include <stdexcept>

// Stop the demonstration if a test fails.
void check(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error("TEST FAILED: " + message);
    }

    std::cout << "PASS: " << message << '\n';
}

// Confirm that a prohibited sale throws our custom exception.
template <typename Operation>
void expectSalesError(Operation operation,
    const std::string& message)
{
    try
    {
        operation();
    }
    catch (const SalesError&)
    {
        std::cout << "PASS: " << message << '\n';
        return;
    }

    throw std::runtime_error("TEST FAILED: " + message);
}

int main()
{
    try
    {
        StadiumShop shop;

        // Prices are in cents and already include GCT.
        shop.addProduct(std::make_unique<Merchandise>(
            "M001", "Team Jersey", 230000, 5,
            "Medium", "Kingston FC"));

        shop.addProduct(std::make_unique<FoodItem>(
            "F001", "Bottled Water", 11500, 3, "Beverage"));

        // Two seats make the capacity boundary easy to test.
        shop.registerFixture("FIX001", 2, 115000);

        std::cout << "\n=== AVAILABLE PRODUCTS ===\n";
        shop.displayProducts(std::cout);

        // Buy a J$2,300 jersey using J$2,500.
        SaleReceipt receipt = shop.purchaseProduct(
            "S001", "M001", 1, 250000);

        receipt.display(std::cout);

        check(receipt.subtotalInCents == 200000,
            "Jersey subtotal is J$2,000.");

        check(receipt.taxInCents == 30000,
            "Included GCT is J$300.");

        check(receipt.changeInCents == 20000,
            "Change is J$200.");

        check(shop.findProduct("M001").getStockQuantity() == 4,
            "Jersey stock decreased from 5 to 4.");

        expectSalesError([&]()
            {
                shop.purchaseProduct("S001", "F001", 1, 100);
            }, "Insufficient payment rejected.");

        expectSalesError([&]()
            {
                shop.purchaseProduct("S001", "F001", 4, 100000);
            }, "Purchase above available stock rejected.");

        expectSalesError([&]()
            {
                shop.purchaseProduct("S001", "F001", 0, 100000);
            }, "Zero purchase quantity rejected.");

        check(shop.findProduct("F001").getStockQuantity() == 3,
            "Failed purchases did not reduce stock.");

        shop.sellTicket(
            "T001", "S001", "FIX001", "General", 115000);

        expectSalesError([&]()
            {
                shop.sellTicket(
                    "T001", "S002", "FIX001", "General", 115000);
            }, "Duplicate ticket ID rejected.");

        shop.sellTicket(
            "T002", "S002", "FIX001", "General", 115000);

        check(shop.getTicketsSold("FIX001") == 2,
            "The final available seat was sold.");

        expectSalesError([&]()
            {
                shop.sellTicket(
                    "T003", "S003", "FIX001", "General", 115000);
            }, "Ticket sale above capacity rejected.");

        // Save, then reload into a separate shop.
        shop.saveData("data/shop_demo_test.txt");

        StadiumShop restored;
        restored.loadData("data/shop_demo_test.txt");

        check(restored.findProduct("M001").getStockQuantity() == 4,
            "Product stock restored from file.");

        check(restored.getTicketsSold("FIX001") == 2,
            "Ticket count restored from file.");

        expectSalesError([&]()
            {
                restored.sellTicket(
                    "T004", "S004", "FIX001", "General", 115000);
            }, "Capacity remains enforced after reloading.");

        // Buy all remaining water with exact payment.
        SaleReceipt waterReceipt = restored.purchaseProduct(
            "S003", "F001", 3, 34500);

        check(waterReceipt.changeInCents == 0,
            "Exact payment produces zero change.");

        check(restored.findProduct("F001").getStockQuantity() == 0,
            "Buying all remaining stock leaves zero.");

        check(waterReceipt.receiptId == "R4",
            "Receipt numbering continues after reloading.");

        restored.displaySalesReport(std::cout);

        std::cout << "\nALL DEMONSTRATION TESTS PASSED.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "\nERROR: " << error.what() << '\n';
        return 1;
    }
}

