#ifndef STADIUMSHOP_H
#define STADIUMSHOP_H

#include "Product.h"
#include "Ticket.h"
#include "league/LeagueRuleError.h"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Custom exception for sales business-rule violations.
class SalesError : public LeagueRuleError
{
public:
    explicit SalesError(const std::string& message)
        : LeagueRuleError(message)
    {
    }
};

// A completed sale's receipt.
struct SaleReceipt
{
    std::string receiptId;
    std::string spectatorId;
    std::string description;

    int quantity = 0;

    std::int64_t subtotalInCents = 0;
    std::int64_t taxInCents = 0;
    std::int64_t totalInCents = 0;
    std::int64_t paymentInCents = 0;
    std::int64_t changeInCents = 0;

    void display(std::ostream& out) const;
};

class StadiumShop
{
private:
    struct FixtureSalesInfo
    {
        int capacity = 0;
        std::int64_t ticketPriceInCents = 0;
    };

    std::map<std::string, std::unique_ptr<Product>> inventory;
    std::map<std::string, FixtureSalesInfo> fixtures;

    std::vector<Ticket> tickets;
    std::vector<SaleReceipt> receipts;

    SaleReceipt prepareReceipt(
        const std::string& spectatorId,
        const std::string& description,
        int quantity,
        std::int64_t unitPriceInCents,
        std::int64_t paymentInCents) const;

public:
    void addProduct(std::unique_ptr<Product> product);

    const Product& findProduct(
        const std::string& productId) const;

    void restockProduct(
        const std::string& productId,
        int quantity);

    void displayProducts(std::ostream& out) const;

    SaleReceipt purchaseProduct(
        const std::string& spectatorId,
        const std::string& productId,
        int quantity,
        std::int64_t paymentInCents);

    void registerFixture(
        const std::string& fixtureId,
        int capacity,
        std::int64_t ticketPriceInCents);

    bool hasFixture(const std::string& fixtureId) const;

    int getTicketsSold(const std::string& fixtureId) const;

    SaleReceipt sellTicket(
        const std::string& ticketId,
        const std::string& spectatorId,
        const std::string& fixtureId,
        const std::string& category,
        std::int64_t paymentInCents);

    void displaySalesReport(std::ostream& out) const;

    void saveData(const std::string& filename) const;
    void loadData(const std::string& filename);
};

#endif