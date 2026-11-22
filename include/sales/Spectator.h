#ifndef SPECTATOR_H
#define SPECTATOR_H

#include "../personnel/Person.h"

class Spectator : public Person
{
public:
    Spectator(const std::string& id,
        const std::string& name,
        int age,
        const std::string& gender);

    std::string getSpectatorId() const;

    void display() const override;
};

#endif