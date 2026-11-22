#ifndef COACH_H
#define COACH_H

#include "Persistable.h"
#include "personnel/Person.h"
#include <string>
#include <vector>

class Coach : public Person, public Persistable {
private:
    std::string qualification;
    std::string coachingRole;
    std::string teamId;

public:
    Coach(const std::string& id, const std::string& name, int age, const std::string& gender,
          const std::string& qualification, const std::string& coachingRole,
          const std::string& teamId = "");

    std::string getQualification() const { return qualification; }
    std::string getCoachingRole() const { return coachingRole; }
    std::string getTeamId() const { return teamId; }

    void setQualification(const std::string& newQualification);
    void setCoachingRole(const std::string& newRole);
    void assignToTeam(const std::string& newTeamId) { teamId = newTeamId; }

    void display() const override;
    std::string serialize() const override;
    static Coach fromRecord(const std::vector<std::string>& f);
};

#endif
