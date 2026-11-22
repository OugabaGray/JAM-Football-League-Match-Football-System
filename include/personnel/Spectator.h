#ifndef SPECTATOR_H
#define SPECTATOR_H

#include "Persistable.h"
#include "personnel/Person.h"
#include <string>
#include <vector>

class Spectator : public Person, public Persistable {
public:
    Spectator(const std::string& id, const std::string& name, int age,
              const std::string& gender);

    void display() const override;
    std::string serialize() const override;
    static Spectator fromRecord(const std::vector<std::string>& f);
};

#endif
