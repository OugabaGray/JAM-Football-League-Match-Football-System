#ifndef STADIUM_H
#define STADIUM_H

#include "Persistable.h"
#include "league/LeagueRuleError.h"
#include <string>
#include <vector>

// Field condition replaces a free-text string so a typo cannot silently change playability.
enum class FieldCondition { Excellent, Good, Fair, Poor, Unplayable };

inline std::string fieldConditionName(FieldCondition condition) {
    switch (condition) {
        case FieldCondition::Excellent: return "Excellent";
        case FieldCondition::Good: return "Good";
        case FieldCondition::Fair: return "Fair";
        case FieldCondition::Poor: return "Poor";
        default: return "Unplayable";
    }
}

// A stadium manages its capacity, location, field condition, ticket price and occupancy.
// Design from Section C (field condition, occupancy, bookSeats), with money kept in cents.
class Stadium : public Persistable {
private:
    std::string id;
    std::string name;
    std::string location;
    int seatingCapacity;
    FieldCondition fieldCondition;
    int ticketPriceCents;
    int currentOccupancy;

    static bool blank(const std::string& text) {
        return text.find_first_not_of(" \t\r\n") == std::string::npos;
    }

public:
    Stadium(const std::string& id, const std::string& name, const std::string& location,
            int seatingCapacity, FieldCondition fieldCondition, int ticketPriceCents,
            int currentOccupancy = 0)
        : id(id), name(name), location(location), seatingCapacity(seatingCapacity),
          fieldCondition(fieldCondition), ticketPriceCents(ticketPriceCents),
          currentOccupancy(currentOccupancy) {
        if (blank(id) || blank(name) || blank(location) || seatingCapacity <= 0) {
            throw LeagueRuleError("A stadium needs an ID, name, location, and positive capacity.");
        }
        if (ticketPriceCents < 0) {
            throw LeagueRuleError("A ticket price cannot be negative.");
        }
        if (currentOccupancy < 0 || currentOccupancy > seatingCapacity) {
            throw LeagueRuleError("Occupancy must be between zero and the seating capacity.");
        }
    }

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::string getLocation() const { return location; }
    int getSeatingCapacity() const { return seatingCapacity; }
    FieldCondition getFieldCondition() const { return fieldCondition; }
    int getTicketPriceCents() const { return ticketPriceCents; }
    int getCurrentOccupancy() const { return currentOccupancy; }
    int getSeatsLeft() const { return seatingCapacity - currentOccupancy; }
    // Fixtures can only be scheduled where the field can be played on.
    bool isPlayable() const { return fieldCondition != FieldCondition::Unplayable; }

    void updateFieldCondition(FieldCondition condition) { fieldCondition = condition; }

    // Ticket sales may never go past the seating capacity (a mandatory business rule).
    void bookSeats(int count) {
        if (count <= 0) {
            throw LeagueRuleError("Ticket quantity must be positive.");
        }
        if (count > getSeatsLeft()) {
            throw LeagueRuleError("Ticket sales cannot exceed the stadium's seating capacity.");
        }
        currentOccupancy += count;
    }

    void resetOccupancy() { currentOccupancy = 0; }

    // Saved as: id|name|location|capacity|condition (0-4)|ticketPriceCents|occupancy
    std::string serialize() const override {
        return Codec::join({id, name, location, std::to_string(seatingCapacity),
                            std::to_string(static_cast<int>(fieldCondition)),
                            std::to_string(ticketPriceCents), std::to_string(currentOccupancy)});
    }

    static Stadium fromRecord(const std::vector<std::string>& f) {
        Codec::requireFields(f, 7, "Stadium");
        int code = Codec::toInt(f[4], "field condition");
        if (code < 0 || code > 4) {
            throw DataFormatException("field condition " + f[4] + " is not 0 to 4");
        }
        return Stadium(f[0], f[1], f[2], Codec::toInt(f[3], "seating capacity"),
                       static_cast<FieldCondition>(code),
                       Codec::toInt(f[5], "ticket price"), Codec::toInt(f[6], "occupancy"));
    }
};

#endif
