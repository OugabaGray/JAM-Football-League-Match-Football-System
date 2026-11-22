#ifndef STADIUM_H
#define STADIUM_H

#include "Persistable.h"
#include <string>
#include <vector>

enum class FieldCondition { Excellent, Good, Fair, Poor, Unplayable };

std::string fieldConditionName(FieldCondition condition);

class Stadium : public Persistable {
private:
    std::string id;
    std::string name;
    std::string location;
    int seatingCapacity;
    FieldCondition fieldCondition;
    int ticketPriceCents;
    int currentOccupancy;

public:
    Stadium(const std::string& id, const std::string& name, const std::string& location,
            int seatingCapacity, FieldCondition fieldCondition, int ticketPriceCents,
            int currentOccupancy = 0);

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::string getLocation() const { return location; }
    int getSeatingCapacity() const { return seatingCapacity; }
    FieldCondition getFieldCondition() const { return fieldCondition; }
    int getTicketPriceCents() const { return ticketPriceCents; }
    int getCurrentOccupancy() const { return currentOccupancy; }
    int getSeatsLeft() const { return seatingCapacity - currentOccupancy; }
    bool isPlayable() const { return fieldCondition != FieldCondition::Unplayable; }

    void updateFieldCondition(FieldCondition condition) { fieldCondition = condition; }
    void bookSeats(int count);
    void resetOccupancy() { currentOccupancy = 0; }

    std::string serialize() const override;
    static Stadium fromRecord(const std::vector<std::string>& f);
};

#endif
