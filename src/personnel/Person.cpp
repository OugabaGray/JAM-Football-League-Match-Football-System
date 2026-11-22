#include "personnel/Person.h"
#include "league/LeagueRuleError.h"

Person::Person(const std::string& id, const std::string& name, int age,
               const std::string& gender)
    : id(id), name(name), age(age), gender(gender) {
    if (id.find_first_not_of(" \t\r\n") == std::string::npos ||
        name.find_first_not_of(" \t\r\n") == std::string::npos ||
        gender.find_first_not_of(" \t\r\n") == std::string::npos || age < 0) {
        throw LeagueRuleError("Enter valid person details.");
    }
}
