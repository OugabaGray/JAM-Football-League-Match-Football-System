#ifndef MATCH_OFFICIAL_H
#define MATCH_OFFICIAL_H

#include "Persistable.h"
#include "personnel/Person.h"
#include <string>
#include <vector>

class MatchOfficial : public Person, public Persistable {
private:
    std::string officiatingRole;
    std::string rank;
    bool available;

public:
    MatchOfficial(const std::string& id, const std::string& name, int age,
                  const std::string& gender, const std::string& officiatingRole = "Referee",
                  const std::string& rank = "Unranked", bool available = true);

    std::string getOfficiatingRole() const { return officiatingRole; }
    std::string getRank() const { return rank; }
    bool isAvailable() const { return available; }
    void setAvailability(bool status) { available = status; }

    void display() const override;
    std::string serialize() const override;
    static MatchOfficial fromRecord(const std::vector<std::string>& f);
};

#endif
