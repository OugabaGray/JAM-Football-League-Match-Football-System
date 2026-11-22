#include "../../include/sales/Spectator.h"

#include <iostream>
#include <stdexcept>

Spectator::Spectator(
    const std::string& id,
    const std::string& name,
    int age,
    const std::string& gender)
    : Person(id, name, age, gender)
{
    if (id.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument(
            "Spectator ID cannot be blank.");
    }

    if (name.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument(
            "Spectator name cannot be blank.");
    }

    if (age < 0)
    {
        throw std::invalid_argument(
            "Spectator age cannot be negative.");
    }

    if (gender.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        throw std::invalid_argument(
            "Spectator gender cannot be blank.");
    }
}

std::string Spectator::getSpectatorId() const
{
    return getId();
}

void Spectator::display() const
{
    // Person's fields are private, so use its public getters.
    std::cout << "\n--- SPECTATOR ---\n"
        << "ID: " << getId() << '\n'
        << "Name: " << getName() << '\n'
        << "Age: " << getAge() << '\n'
        << "Gender: " << getGender() << '\n';
}