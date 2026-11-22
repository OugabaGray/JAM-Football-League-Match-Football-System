#ifndef COACH_H
#define COACH_H

#include "Persistable.h"
#include "personnel/Person.h"
#include <iostream>
#include <string>
#include <vector>

class Coach : public Person, public Persistable {
private:
    std::string qualification;
    std::string coachingRole;
    std::string teamId;

    static bool blank(const std::string& text) {
        return text.find_first_not_of(" \t\r\n") == std::string::npos;
    }

public:
    Coach(const std::string& id, const std::string& name, int age, const std::string& gender,
          const std::string& qualification, const std::string& coachingRole,
          const std::string& teamId = "")
        : Person(id, name, age, gender), qualification(qualification),
          coachingRole(coachingRole), teamId(teamId) {
        if (blank(qualification) || blank(coachingRole)) {
            throw LeagueRuleError("A coach needs a qualification and a coaching role.");
        }
    }

    std::string getQualification() const { return qualification; }
    std::string getCoachingRole() const { return coachingRole; }
    std::string getTeamId() const { return teamId; }

    void setQualification(const std::string& newQualification) {
        if (blank(newQualification)) {
            throw LeagueRuleError("A coach needs a qualification.");
        }
        qualification = newQualification;
    }

    void setCoachingRole(const std::string& newRole) {
        if (blank(newRole)) {
            throw LeagueRuleError("A coach needs a coaching role.");
        }
        coachingRole = newRole;
    }

    void assignToTeam(const std::string& newTeamId) { teamId = newTeamId; }

    void display() const override {
        std::cout << getName() << " (Coach " << getId() << ") | " << coachingRole << " | "
                  << qualification << " | Age " << getAge() << " | " << getGender() << '\n';
    }

    std::string serialize() const override {
        return Codec::join({getId(), getName(), std::to_string(getAge()), getGender(),
                            qualification, coachingRole, teamId});
    }

    static Coach fromRecord(const std::vector<std::string>& f) {
        Codec::requireFields(f, 7, "Coach");
        return Coach(f[0], f[1], Codec::toInt(f[2], "age"), f[3], f[4], f[5], f[6]);
    }
};

#endif
