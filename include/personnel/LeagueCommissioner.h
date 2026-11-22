#ifndef LEAGUE_COMMISSIONER_H
#define LEAGUE_COMMISSIONER_H

#include "personnel/Person.h"
#include <string>

class FootballLeague;

// A commissioner is a Person, so identity fields come from the base class.
class LeagueCommissioner : public Person {
public:
    LeagueCommissioner(const std::string& id, const std::string& name, int age,
                       const std::string& gender);

    // override asks the compiler to check that this matches Person::display().
    void display() const override;
    void approveRegistration(FootballLeague& league, const std::string& teamId) const;
    void suspendRegistration(FootballLeague& league, const std::string& teamId) const;
    void assignMatchOfficial(FootballLeague& league, const std::string& fixtureId,
                             const std::string& officialId) const;
    std::string inspectMatchPreparations(const FootballLeague& league,
                                         const std::string& fixtureId) const;
};

#endif
