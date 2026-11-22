#include "../../include/sales/StadiumShop.h"
#include "../../include/sales/Merchandise.h"
#include "../../include/sales/FoodItem.h"

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

// Helper functions used only inside this file.
namespace
{
    bool isBlank(const std::string& text)
    {
        return text.find_first_not_of(" \t\r\n")
            == std::string::npos;
    }

    std::string formatMoney(std::int64_t cents)
    {
        std::ostringstream amount;

        amount << cents / 100 << '.'
            << std::setfill('0') << std::setw(2)
            << cents % 100;

        return amount.str();
    }
}

void SaleReceipt::display(std::ostream& out) const
{
    out << "\n--- STADIUM SALES RECEIPT ---\n"
        << "Receipt ID: " << receiptId << '\n'
        << "Spectator ID: " << spectatorId << '\n'
        << "Item: " << description << '\n'
        << "Quantity: " << quantity << '\n'
        << "Subtotal: J$"
        << formatMoney(subtotalInCents) << '\n'
        << "GCT included (15%): J$"
        << formatMoney(taxInCents) << '\n'
        << "Total: J$"
        << formatMoney(totalInCents) << '\n'
        << "Payment: J$"
        << formatMoney(paymentInCents) << '\n'
        << "Change: J$"
        << formatMoney(changeInCents) << '\n'
        << "Thank you for your purchase!\n";
}

void StadiumShop::addProduct(std::unique_ptr<Product> product)
{
    if (!product)
    {
        throw SalesError("Cannot add an empty product.");
    }

    const std::string id = product->getProductId();

    if (inventory.find(id) != inventory.end())
    {
        throw SalesError("Product ID already exists.");
    }

    inventory.emplace(id, std::move(product));
}

const Product& StadiumShop::findProduct(
    const std::string& productId) const
{
    const auto item = inventory.find(productId);

    if (item == inventory.end())
    {
        throw SalesError("Product not found.");
    }

    return *(item->second);
}

void StadiumShop::restockProduct(
    const std::string& productId,
    int quantity)
{
    const auto item = inventory.find(productId);

    if (item == inventory.end())
    {
        throw SalesError("Product not found.");
    }

    if (quantity <= 0)
    {
        throw SalesError("Restock quantity must be positive.");
    }

    item->second->restock(quantity);
}

void StadiumShop::displayProducts(std::ostream& out) const
{
    if (inventory.empty())
    {
        out << "No products available.\n";
        return;
    }

    for (const auto& item : inventory)
    {
        item.second->display(out);
        out << '\n';
    }
}

SaleReceipt StadiumShop::prepareReceipt(
    const std::string& spectatorId,
    const std::string& description,
    int quantity,
    std::int64_t unitPriceInCents,
    std::int64_t paymentInCents) const
{
    if (isBlank(spectatorId))
    {
        throw SalesError("Spectator ID cannot be blank.");
    }

    if (isBlank(description))
    {
        throw SalesError("Sale description cannot be blank.");
    }

    if (quantity <= 0)
    {
        throw SalesError("Purchase quantity must be positive.");
    }

    if (unitPriceInCents < 0 || paymentInCents < 0)
    {
        throw SalesError("Price and payment cannot be negative.");
    }

    const auto maximum =
        (std::numeric_limits<std::int64_t>::max)();

    if (unitPriceInCents > maximum / quantity)
    {
        throw SalesError("Purchase total is too large.");
    }

    const std::int64_t total = unitPriceInCents * quantity;

    if (paymentInCents < total)
    {
        throw SalesError("Insufficient payment.");
    }

    SaleReceipt receipt;

    receipt.receiptId =
        "R" + std::to_string(receipts.size() + 1);

    receipt.spectatorId = spectatorId;
    receipt.description = description;
    receipt.quantity = quantity;
    receipt.totalInCents = total;

    // Prices already include 15% GCT.
    // Tax = total * 15 / 115, rounded to the nearest cent.
    // Splitting the calculation avoids integer overflow.
    receipt.taxInCents =
        (total / 115) * 15
        + ((total % 115) * 15 + 57) / 115;

    receipt.subtotalInCents =
        total - receipt.taxInCents;

    receipt.paymentInCents = paymentInCents;
    receipt.changeInCents = paymentInCents - total;

    return receipt;
}

SaleReceipt StadiumShop::purchaseProduct(
    const std::string& spectatorId,
    const std::string& productId,
    int quantity,
    std::int64_t paymentInCents)
{
    const auto item = inventory.find(productId);

    if (item == inventory.end())
    {
        throw SalesError("Product not found.");
    }

    if (quantity <= 0)
    {
        throw SalesError("Purchase quantity must be positive.");
    }

    Product& product = *(item->second);

    if (quantity > product.getStockQuantity())
    {
        throw SalesError("Insufficient stock.");
    }

    // Check payment and calculate totals before changing stock.
    SaleReceipt receipt = prepareReceipt(
        spectatorId,
        product.getName(),
        quantity,
        product.getPriceInCents(),
        paymentInCents);

    // Record the sale before reducing the validated stock.
    receipts.push_back(receipt);
    product.reduceStock(quantity);

    return receipt;
}

void StadiumShop::displaySalesReport(std::ostream& out) const
{
    // Report 1: Revenue and tax from all completed sales.
    std::int64_t totalRevenue = 0;
    std::int64_t totalTax = 0;

    const auto maximum =
        (std::numeric_limits<std::int64_t>::max)();

    for (const SaleReceipt& receipt : receipts)
    {
        if (receipt.totalInCents > maximum - totalRevenue
            || receipt.taxInCents > maximum - totalTax)
        {
            throw SalesError("Report totals are too large.");
        }

        totalRevenue += receipt.totalInCents;
        totalTax += receipt.taxInCents;
    }

    out << "\n=== SALES SUMMARY ===\n"
        << "Completed transactions: " << receipts.size() << '\n'
        << "Revenue before GCT: J$"
        << formatMoney(totalRevenue - totalTax) << '\n'
        << "GCT included: J$"
        << formatMoney(totalTax) << '\n'
        << "Total revenue (tickets and products): J$"
        << formatMoney(totalRevenue) << '\n';

    // Report 2: Tickets sold and seats remaining per fixture.
    out << "\n=== TICKET SALES BY FIXTURE ===\n";

    if (fixtures.empty())
    {
        out << "No fixtures registered for sales.\n";
    }

    for (const auto& entry : fixtures)
    {
        const int sold = getTicketsSold(entry.first);
        const int capacity = entry.second.capacity;

        out << "Fixture: " << entry.first << '\n'
            << "Capacity: " << capacity << '\n'
            << "Tickets sold: " << sold << '\n'
            << "Seats remaining: " << capacity - sold << "\n\n";
    }

    // Report 3: Products with five or fewer units remaining.
    out << "\n=== LOW-STOCK REPORT ===\n";

    bool foundLowStock = false;

    for (const auto& entry : inventory)
    {
        const Product& product = *(entry.second);

        if (product.getStockQuantity() <= 5)
        {
            foundLowStock = true;

            out << "ID: " << product.getProductId()
                << " | Name: " << product.getName()
                << " | Stock: " << product.getStockQuantity()
                << '\n';
        }
    }

    if (!foundLowStock)
    {
        out << "No products are low in stock.\n";
    }
}

void StadiumShop::saveData(const std::string& filename) const
{
    if (isBlank(filename))
    {
        throw SalesError("Filename cannot be blank.");
    }

    // Prepare the data before opening the destination file.
    std::ostringstream data;

    data << "KPFL_SHOP_V1\n";

    // Save products, including their specific subclass details.
    data << "PRODUCTS " << inventory.size() << '\n';

    for (const auto& entry : inventory)
    {
        const Product& product = *(entry.second);

        const auto* merchandise =
            dynamic_cast<const Merchandise*>(&product);

        const auto* food =
            dynamic_cast<const FoodItem*>(&product);

        if (!merchandise && !food)
        {
            throw SalesError("Unsupported product type.");
        }

        data << std::quoted(product.getType()) << ' '
            << std::quoted(product.getProductId()) << ' '
            << std::quoted(product.getName()) << ' '
            << product.getPriceInCents() << ' '
            << product.getStockQuantity() << ' ';

        if (merchandise)
        {
            data << std::quoted(merchandise->getSize()) << ' '
                << std::quoted(merchandise->getTeamName());
        }
        else
        {
            data << std::quoted(food->getCategory());
        }

        data << '\n';
    }

    // Save each fixture's capacity and ticket price.
    data << "FIXTURES " << fixtures.size() << '\n';

    for (const auto& entry : fixtures)
    {
        data << std::quoted(entry.first) << ' '
            << entry.second.capacity << ' '
            << entry.second.ticketPriceInCents << '\n';
    }

    // Save issued tickets so capacity counts survive a restart.
    data << "TICKETS " << tickets.size() << '\n';

    for (const Ticket& ticket : tickets)
    {
        data << std::quoted(ticket.getTicketId()) << ' '
            << std::quoted(ticket.getSpectatorId()) << ' '
            << std::quoted(ticket.getFixtureId()) << ' '
            << std::quoted(ticket.getCategory()) << ' '
            << ticket.getPriceInCents() << '\n';
    }

    // Save completed receipts and their original money values.
    data << "RECEIPTS " << receipts.size() << '\n';

    for (const SaleReceipt& receipt : receipts)
    {
        data << std::quoted(receipt.receiptId) << ' '
            << std::quoted(receipt.spectatorId) << ' '
            << std::quoted(receipt.description) << ' '
            << receipt.quantity << ' '
            << receipt.subtotalInCents << ' '
            << receipt.taxInCents << ' '
            << receipt.totalInCents << ' '
            << receipt.paymentInCents << ' '
            << receipt.changeInCents << '\n';
    }

    data << "END\n";

    if (!data)
    {
        throw SalesError("Could not prepare data for saving.");
    }

    std::ofstream file(filename);

    if (!file)
    {
        throw SalesError("Could not open file for saving.");
    }

    file << data.str();
    file.close();

    if (!file)
    {
        throw SalesError("Could not finish saving data.");
    }
}

void StadiumShop::loadData(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        throw SalesError("Could not open file for loading.");
    }

    std::string heading;

    if (!(file >> heading) || heading != "KPFL_SHOP_V1")
    {
        throw SalesError("Invalid shop data file.");
    }

    // Load into a temporary shop.
    // Existing data is replaced only after validation succeeds.
    StadiumShop loaded;

    auto readCount = [&](const std::string& expected) -> int
        {
            std::string section;
            long long count = 0;

            if (!(file >> section >> count)
                || section != expected
                || count < 0
                || count > 1000000)
            {
                throw SalesError("Invalid section: " + expected);
            }

            return static_cast<int>(count);
        };

    // Read products and recreate the correct subclasses.
    const int productCount = readCount("PRODUCTS");

    for (int i = 0; i < productCount; ++i)
    {
        std::string type;
        std::string id;
        std::string name;
        std::int64_t price = 0;
        int stock = 0;

        if (!(file >> std::quoted(type)
            >> std::quoted(id)
            >> std::quoted(name)
            >> price >> stock))
        {
            throw SalesError("Invalid product record.");
        }

        if (type == "Merchandise")
        {
            std::string size;
            std::string team;

            if (!(file >> std::quoted(size)
                >> std::quoted(team)))
            {
                throw SalesError("Invalid merchandise record.");
            }

            loaded.addProduct(std::make_unique<Merchandise>(
                id, name, price, stock, size, team));
        }
        else if (type == "FoodItem")
        {
            std::string category;

            if (!(file >> std::quoted(category)))
            {
                throw SalesError("Invalid food record.");
            }

            loaded.addProduct(std::make_unique<FoodItem>(
                id, name, price, stock, category));
        }
        else
        {
            throw SalesError("Unknown saved product type.");
        }
    }

    // Read fixture capacities and ticket prices.
    const int fixtureCount = readCount("FIXTURES");

    for (int i = 0; i < fixtureCount; ++i)
    {
        std::string id;
        int capacity = 0;
        std::int64_t price = 0;

        if (!(file >> std::quoted(id) >> capacity >> price))
        {
            throw SalesError("Invalid fixture record.");
        }

        loaded.registerFixture(id, capacity, price);
    }

    // Restore tickets without processing another payment.
    const int ticketCount = readCount("TICKETS");
    std::map<std::string, bool> ticketIds;

    for (int i = 0; i < ticketCount; ++i)
    {
        std::string id;
        std::string spectatorId;
        std::string fixtureId;
        std::string category;
        std::int64_t price = 0;

        if (!(file >> std::quoted(id)
            >> std::quoted(spectatorId)
            >> std::quoted(fixtureId)
            >> std::quoted(category)
            >> price))
        {
            throw SalesError("Invalid ticket record.");
        }

        const auto fixture = loaded.fixtures.find(fixtureId);

        if (fixture == loaded.fixtures.end())
        {
            throw SalesError("Saved ticket has an unknown fixture.");
        }

        if (!ticketIds.emplace(id, true).second)
        {
            throw SalesError("Duplicate saved ticket ID.");
        }

        if (loaded.getTicketsSold(fixtureId)
            >= fixture->second.capacity)
        {
            throw SalesError("Saved tickets exceed stadium capacity.");
        }

        loaded.tickets.emplace_back(
            id, spectatorId, fixtureId, category, price);
    }

    // Restore receipts without reducing stock again.
    const int receiptCount = readCount("RECEIPTS");

    for (int i = 0; i < receiptCount; ++i)
    {
        SaleReceipt receipt;

        if (!(file >> std::quoted(receipt.receiptId)
            >> std::quoted(receipt.spectatorId)
            >> std::quoted(receipt.description)
            >> receipt.quantity
            >> receipt.subtotalInCents
            >> receipt.taxInCents
            >> receipt.totalInCents
            >> receipt.paymentInCents
            >> receipt.changeInCents))
        {
            throw SalesError("Invalid receipt record.");
        }

        // IDs must match the sequence used when creating receipts.
        if (receipt.receiptId != "R" + std::to_string(i + 1)
            || isBlank(receipt.spectatorId)
            || isBlank(receipt.description)
            || receipt.quantity <= 0)
        {
            throw SalesError("Invalid saved receipt details.");
        }

        const auto total = receipt.totalInCents;
        const auto tax = receipt.taxInCents;
        const auto payment = receipt.paymentInCents;

        if (total < 0 || tax < 0 || tax > total
            || payment < total)
        {
            throw SalesError("Invalid saved payment amounts.");
        }

        const std::int64_t expectedTax =
            (total / 115) * 15
            + ((total % 115) * 15 + 57) / 115;

        if (tax != expectedTax
            || receipt.subtotalInCents != total - tax
            || receipt.changeInCents != payment - total)
        {
            throw SalesError("Saved receipt totals do not match.");
        }

        loaded.receipts.push_back(receipt);
    }

    if (ticketCount > receiptCount)
    {
        throw SalesError("Saved tickets are missing sales receipts.");
    }

    if (!(file >> heading) || heading != "END")
    {
        throw SalesError("Shop data file is incomplete.");
    }

    file >> std::ws;

    if (file.bad() || !file.eof())
    {
        throw SalesError("Unexpected data at end of file.");
    }

    // All checks passed: replace the current shop data.
    inventory.swap(loaded.inventory);
    fixtures.swap(loaded.fixtures);
    tickets.swap(loaded.tickets);
    receipts.swap(loaded.receipts);
}

// Register a fixture for ticket sales.
void StadiumShop::registerFixture(
    const std::string& fixtureId,
    int capacity,
    std::int64_t ticketPriceInCents)
{
    if (isBlank(fixtureId))
    {
        throw SalesError("Fixture ID cannot be blank.");
    }

    if (capacity <= 0)
    {
        throw SalesError("Stadium capacity must be positive.");
    }

    if (ticketPriceInCents < 0)
    {
        throw SalesError("Ticket price cannot be negative.");
    }

    if (fixtures.find(fixtureId) != fixtures.end())
    {
        throw SalesError("Fixture is already registered for sales.");
    }

    FixtureSalesInfo info;
    info.capacity = capacity;
    info.ticketPriceInCents = ticketPriceInCents;

    fixtures.emplace(fixtureId, info);
}

// Count issued tickets for one specific fixture.
int StadiumShop::getTicketsSold(
    const std::string& fixtureId) const
{
    if (fixtures.find(fixtureId) == fixtures.end())
    {
        throw SalesError("Fixture not found.");
    }

    int count = 0;

    for (const Ticket& ticket : tickets)
    {
        if (ticket.getFixtureId() == fixtureId)
        {
            ++count;
        }
    }

    return count;
}

// Validate payment and capacity before issuing a ticket.
SaleReceipt StadiumShop::sellTicket(
    const std::string& ticketId,
    const std::string& spectatorId,
    const std::string& fixtureId,
    const std::string& category,
    std::int64_t paymentInCents)
{
    if (isBlank(ticketId))
    {
        throw SalesError("Ticket ID cannot be blank.");
    }

    if (isBlank(category))
    {
        throw SalesError("Ticket category cannot be blank.");
    }

    const auto fixture = fixtures.find(fixtureId);

    if (fixture == fixtures.end())
    {
        throw SalesError("Fixture not found.");
    }

    for (const Ticket& ticket : tickets)
    {
        if (ticket.getTicketId() == ticketId)
        {
            throw SalesError("Ticket ID already exists.");
        }
    }

    if (getTicketsSold(fixtureId) >= fixture->second.capacity)
    {
        throw SalesError("Sold out: stadium capacity reached.");
    }

    SaleReceipt receipt = prepareReceipt(
        spectatorId,
        "Ticket " + ticketId + " | Fixture "
        + fixtureId + " | " + category,
        1,
        fixture->second.ticketPriceInCents,
        paymentInCents);

    Ticket ticket(
        ticketId,
        spectatorId,
        fixtureId,
        category,
        fixture->second.ticketPriceInCents);

    tickets.push_back(ticket);

    try
    {
        receipts.push_back(receipt);
    }
    catch (...)
    {
        // Undo the ticket if recording its receipt fails.
        tickets.pop_back();
        throw;
    }

    return receipt;
}