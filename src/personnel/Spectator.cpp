#include "personnel/Spectator.h"
#include <iostream>

Spectator::Spectator(const std::string& id, const std::string& name, int age,
                     const std::string& gender)
    : Person(id, name, age, gender) {}

void Spectator::display() const {
    std::cout << getName() << " (Spectator " << getId() << ")\n";
}

std::string Spectator::serialize() const {
    return Codec::join({getId(), getName(), std::to_string(getAge()), getGender()});
}

Spectator Spectator::fromRecord(const std::vector<std::string>& f) {
    Codec::requireFields(f, 4, "Spectator");
    return Spectator(f[0], f[1], Codec::toInt(f[2], "age"), f[3]);
}
