#ifndef MATCH_OFFICIAL_H
#define MATCH_OFFICIAL_H

#include "Persistable.h"
#include "personnel/Person.h"
#include <iostream>
#include <vector>

// A match official is a Person with a role, a rank and an availability flag.
// This is the second Person role used to show how virtual display() works.
class MatchOfficial : public Person, public Persistable {
private:
    std::string officiatingRole; // e.g. Referee, Assistant Referee
    std::string rank;            // e.g. Grade 1
    bool available;

public:
    MatchOfficial(const std::string& id, const std::string& name, int age,
                  const std::string& gender, const std::string& officiatingRole = "Referee",
                  const std::string& rank = "Unranked", bool available = true)
        : Person(id, name, age, gender), officiatingRole(officiatingRole), rank(rank),
          available(available) {
        if (officiatingRole.find_first_not_of(" \t\r\n") == std::string::npos ||
            rank.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw LeagueRuleError("A match official needs a role and a rank.");
        }
    }

    std::string getOfficiatingRole() const { return officiatingRole; }
    std::string getRank() const { return rank; }
    bool isAvailable() const { return available; }
    void setAvailability(bool status) { available = status; }

    void display() const override {
        std::cout << getName() << " (Match Official " << getId() << ", " << officiatingRole
                  << ", " << rank << (available ? "" : ", unavailable") << ")\n";
    }

    // Saved as: id|name|age|gender|role|rank|available (1 or 0)
    std::string serialize() const override {
        return Codec::join({getId(), getName(), std::to_string(getAge()), getGender(),
                            officiatingRole, rank, available ? "1" : "0"});
    }

    static MatchOfficial fromRecord(const std::vector<std::string>& f) {
        Codec::requireFields(f, 7, "MatchOfficial");
        return MatchOfficial(f[0], f[1], Codec::toInt(f[2], "age"), f[3], f[4], f[5],
                             Codec::toBool(f[6]));
    }
};

#endif
