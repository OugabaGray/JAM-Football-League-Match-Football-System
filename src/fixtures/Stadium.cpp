#include "fixtures/Stadium.h"
#include "league/LeagueRuleError.h"

namespace {
bool blank(const std::string& text) {
    return text.find_first_not_of(" \t\r\n") == std::string::npos;
}
}

std::string fieldConditionName(FieldCondition condition) {
    switch (condition) {
        case FieldCondition::Excellent: return "Excellent";
        case FieldCondition::Good: return "Good";
        case FieldCondition::Fair: return "Fair";
        case FieldCondition::Poor: return "Poor";
        default: return "Unplayable";
    }
}

Stadium::Stadium(const std::string& id, const std::string& name, const std::string& location,
                 int seatingCapacity, FieldCondition fieldCondition, int ticketPriceCents,
                 int currentOccupancy)
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

void Stadium::bookSeats(int count) {
    if (count <= 0) {
        throw LeagueRuleError("Ticket quantity must be positive.");
    }
    if (count > getSeatsLeft()) {
        throw LeagueRuleError("Ticket sales cannot exceed the stadium's seating capacity.");
    }
    currentOccupancy += count;
}

std::string Stadium::serialize() const {
    return Codec::join({id, name, location, std::to_string(seatingCapacity),
                        std::to_string(static_cast<int>(fieldCondition)),
                        std::to_string(ticketPriceCents), std::to_string(currentOccupancy)});
}

Stadium Stadium::fromRecord(const std::vector<std::string>& f) {
    Codec::requireFields(f, 7, "Stadium");
    int code = Codec::toInt(f[4], "field condition");
    if (code < 0 || code > 4) {
        throw DataFormatException("field condition " + f[4] + " is not 0 to 4");
    }
    return Stadium(f[0], f[1], f[2], Codec::toInt(f[3], "seating capacity"),
                   static_cast<FieldCondition>(code),
                   Codec::toInt(f[5], "ticket price"), Codec::toInt(f[6], "occupancy"));
}
