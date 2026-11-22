#include "../../include/sales/SalesMenu.h"
#include "../../include/sales/Spectator.h"
#include "../../include/sales/Merchandise.h"
#include "../../include/sales/FoodItem.h"

#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iomanip>
#include <utility>

namespace
{
    // Read text, remove surrounding spaces, and reject blank input.
    std::string readText(const std::string& prompt)
    {
        while (true)
        {
            std::cout << prompt;

            std::string text;

            if (!std::getline(std::cin, text))
            {
                throw std::runtime_error("Input stream closed.");
            }

            const auto first = text.find_first_not_of(" \t\r\n");

            if (first == std::string::npos)
            {
                std::cout << "Please enter a value.\n";
                continue;
            }

            const auto last = text.find_last_not_of(" \t\r\n");
            return text.substr(first, last - first + 1);
        }
    }

    // Read a whole number within the permitted range.
    int readInteger(
        const std::string& prompt,
        int minimum,
        int maximum)
    {
        while (true)
        {
            const std::string text = readText(prompt);
            std::istringstream input(text);

            int value = 0;
            char extra = '\0';

            if ((input >> value)
                && !(input >> extra)
                && value >= minimum
                && value <= maximum)
            {
                return value;
            }

            std::cout << "Enter a whole number from "
                << minimum << " to " << maximum << ".\n";
        }
    }

    // Accept dollars, such as 2500 or 2500.50.
    // Convert to whole cents without floating-point rounding.
    std::int64_t readMoney(const std::string& prompt)
    {
        while (true)
        {
            const std::string text = readText(prompt);
            const auto dot = text.find('.');

            const std::string dollars = text.substr(0, dot);

            const std::string fraction =
                dot == std::string::npos
                ? ""
                : text.substr(dot + 1);

            if (dollars.empty()
                || dollars.find_first_not_of("0123456789")
                != std::string::npos
                || fraction.size() > 2
                || fraction.find_first_not_of("0123456789")
                != std::string::npos)
            {
                std::cout
                    << "Enter an amount such as 2500 or 2500.50,"
                    << " without commas or currency symbols.\n";
                continue;
            }

            try
            {
                const std::int64_t whole = std::stoll(dollars);

                std::int64_t cents = 0;

                if (!fraction.empty())
                {
                    cents = fraction[0] - '0';

                    if (fraction.size() == 1)
                    {
                        cents *= 10;
                    }
                    else
                    {
                        cents = cents * 10 + fraction[1] - '0';
                    }
                }

                const auto maximum =
                    (std::numeric_limits<std::int64_t>::max)();

                if (whole > (maximum - cents) / 100)
                {
                    throw std::out_of_range("Amount too large.");
                }

                return whole * 100 + cents;
            }
            catch (const std::out_of_range&)
            {
                std::cout << "That amount is too large.\n";
            }
        }
    }
}

// Save registered spectators in a companion file.
static void saveSpectators(
    const std::string& filename,
    const std::map<std::string, Spectator>& spectators)
{
    std::ofstream file(filename);

    if (!file)
    {
        throw SalesError("Cannot open spectator file for saving.");
    }

    file << "KPFL_SPECTATORS_V1\n"
        << spectators.size() << '\n';

    for (const auto& entry : spectators)
    {
        const Spectator& person = entry.second;

        file << std::quoted(person.getId()) << ' '
            << std::quoted(person.getName()) << ' '
            << person.getAge() << ' '
            << std::quoted(person.getGender()) << '\n';
    }

    file << "END\n";
    file.close();

    if (!file)
    {
        throw SalesError("Could not finish saving spectators.");
    }
}

// Read and validate spectators before changing the current registry.
static std::map<std::string, Spectator> loadSpectators(
    const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        throw SalesError("Cannot open spectator file for loading.");
    }

    std::string heading;
    long long count = 0;

    if (!(file >> heading >> count)
        || heading != "KPFL_SPECTATORS_V1"
        || count < 0
        || count > 1000000)
    {
        throw SalesError("Invalid spectator file.");
    }

    std::map<std::string, Spectator> loaded;

    for (long long i = 0; i < count; ++i)
    {
        std::string id;
        std::string name;
        std::string gender;
        int age = 0;

        if (!(file >> std::quoted(id)
            >> std::quoted(name)
            >> age
            >> std::quoted(gender)))
        {
            throw SalesError("Invalid spectator record.");
        }

        const auto result = loaded.emplace(
            id, Spectator(id, name, age, gender));

        if (!result.second)
        {
            throw SalesError("Duplicate saved spectator ID.");
        }
    }

    if (!(file >> heading) || heading != "END")
    {
        throw SalesError("Spectator file is incomplete.");
    }

    file >> std::ws;

    if (file.bad() || !file.eof())
    {
        throw SalesError("Unexpected data in spectator file.");
    }

    return loaded;
}
void runSalesMenu(StadiumShop& shop)
{
    // Spectators registered during this menu session.
    std::map<std::string, Spectator> spectators;

    const int maximum = (std::numeric_limits<int>::max)();

    try
    {
        while (true)
        {
            std::cout
                << "\n=== TICKETING AND STADIUM SHOP ===\n"
                << "1. Register spectator\n"
                << "2. Display spectators\n"
                << "3. Display products\n"
                << "4. Purchase product\n"
                << "5. Purchase ticket\n"
                << "6. Restock product\n"
                << "7. Add merchandise\n"
                << "8. Add food or beverage\n"
                << "9. Register fixture for ticket sales\n"
                << "10. View reports\n"
                << "11. Save shop data\n"
                << "12. Load shop data\n"
                << "0. Return\n";

            const int choice = readInteger("Choice: ", 0, 12);

            if (choice == 0)
            {
                return;
            }

            try
            {
                switch (choice)
                {
                case 1:
                {
                    const std::string id =
                        readText("Spectator ID: ");

                    if (spectators.find(id) != spectators.end())
                    {
                        throw SalesError(
                            "Spectator ID already registered.");
                    }

                    const std::string name =
                        readText("Full name: ");

                    const int age =
                        readInteger("Age: ", 0, maximum);

                    const std::string gender =
                        readText("Gender: ");

                    spectators.emplace(
                        id, Spectator(id, name, age, gender));

                    std::cout << "Spectator registered.\n";
                    break;
                }

                case 2:
                {
                    if (spectators.empty())
                    {
                        std::cout << "No spectators registered.\n";
                    }

                    for (const auto& entry : spectators)
                    {
                        // Demonstrate polymorphism through Person.
                        const Person& person = entry.second;
                        person.display();
                    }

                    break;
                }

                case 3:
                    shop.displayProducts(std::cout);
                    break;

                case 4:
                case 5:
                {
                    const std::string spectatorId =
                        readText("Registered spectator ID: ");

                    const auto customer =
                        spectators.find(spectatorId);

                    if (customer == spectators.end())
                    {
                        throw SalesError(
                            "Register the spectator first using option 1.");
                    }

                    if (choice == 4)
                    {
                        const std::string productId =
                            readText("Product ID: ");

                        // Show the product and price before payment.
                        shop.findProduct(productId).display(std::cout);

                        const int quantity = readInteger(
                            "Quantity: ", 1, maximum);

                        const std::int64_t payment =
                            readMoney("Payment received (J$): ");

                        const SaleReceipt receipt =
                            shop.purchaseProduct(
                                customer->second.getSpectatorId(),
                                productId,
                                quantity,
                                payment);

                        receipt.display(std::cout);
                    }
                    else
                    {
                        const std::string ticketId =
                            readText("New ticket ID: ");

                        const std::string fixtureId =
                            readText("Fixture ID: ");

                        const std::string category =
                            readText("Ticket category: ");

                        const std::int64_t payment =
                            readMoney("Payment received (J$): ");

                        const SaleReceipt receipt =
                            shop.sellTicket(
                                ticketId,
                                customer->second.getSpectatorId(),
                                fixtureId,
                                category,
                                payment);

                        receipt.display(std::cout);
                    }

                    break;
                }

                case 6:
                {
                    const std::string id =
                        readText("Product ID: ");

                    const int quantity = readInteger(
                        "Quantity to add: ", 1, maximum);

                    shop.restockProduct(id, quantity);

                    std::cout << "Stock updated.\n";
                    break;
                }

                case 7:
                case 8:
                {
                    const std::string id =
                        readText("New product ID: ");

                    const std::string name =
                        readText("Product name: ");

                    const std::int64_t price =
                        readMoney("Unit price including GCT (J$): ");

                    const int stock = readInteger(
                        "Opening stock: ", 0, maximum);

                    if (choice == 7)
                    {
                        const std::string size =
                            readText("Size (or N/A): ");

                        const std::string team =
                            readText("Team name (or N/A): ");

                        shop.addProduct(
                            std::make_unique<Merchandise>(
                                id, name, price, stock, size, team));
                    }
                    else
                    {
                        const int categoryChoice = readInteger(
                            "Category: 1 = Food, 2 = Beverage: ",
                            1, 2);

                        const std::string category =
                            categoryChoice == 1
                            ? "Food" : "Beverage";

                        shop.addProduct(
                            std::make_unique<FoodItem>(
                                id, name, price, stock, category));
                    }

                    std::cout << "Product added.\n";
                    break;
                }

                case 9:
                {
                    const std::string id =
                        readText("Fixture ID: ");

                    const int capacity = readInteger(
                        "Stadium seating capacity: ", 1, maximum);

                    const std::int64_t price =
                        readMoney("Ticket price including GCT (J$): ");

                    shop.registerFixture(id, capacity, price);

                    std::cout << "Fixture registered for sales.\n";
                    break;
                }

                case 10:
                    shop.displaySalesReport(std::cout);
                    break;

                case 11:
                {
                    const std::string filename = readText(
                        "Save filename (e.g. data/shop.txt): ");

                    saveSpectators(
                        filename + ".spectators", spectators);

                    shop.saveData(filename);

                    std::cout
                        << "Shop data and spectators saved.\n";
                    break;
                }

                case 12:
                {
                    const std::string filename = readText(
                        "Load filename (e.g. data/shop.txt): ");

                    const int confirmation = readInteger(
                        "Replace current data? 1 = Yes, 0 = Cancel: ",
                        0, 1);

                    if (confirmation == 1)
                    {
                        auto loadedSpectators = loadSpectators(
                            filename + ".spectators");

                        StadiumShop loadedShop;
                        loadedShop.loadData(filename);

                        shop = std::move(loadedShop);
                        spectators.swap(loadedSpectators);

                        std::cout
                            << "Shop data and spectators loaded.\n";
                    }

                    break;
                }
                } // End switch.
            } // End the operation's try block.
            catch (const std::exception& error)
            {
                if (!std::cin)
                {
                    throw;
                }

                std::cout << "Operation failed: "
                    << error.what() << '\n';
            }
        } // End menu loop.
    } // End outer try block.
    catch (const std::exception& error)
    {
        std::cout << "Menu closed: " << error.what() << '\n';
    }
} // End runSalesMenu.