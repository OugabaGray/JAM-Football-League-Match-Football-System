#ifndef SPECTATOR_H
#define SPECTATOR_H

#include "Persistable.h"
#include "personnel/Person.h"
#include <iostream>
#include <string>
#include <vector>

class Spectator : public Person, public Persistable {
public:
    Spectator(const std::string& id, const std::string& name, int age, const std::string& gender)
        : Person(id, name, age, gender) {}

    void display() const override {
        std::cout << getName() << " (Spectator " << getId() << ")\n";
    }

    std::string serialize() const override {
        return Codec::join({getId(), getName(), std::to_string(getAge()), getGender()});
    }

    static Spectator fromRecord(const std::vector<std::string>& f) {
        Codec::requireFields(f, 4, "Spectator");
        return Spectator(f[0], f[1], Codec::toInt(f[2], "age"), f[3]);
    }
};

#endif
