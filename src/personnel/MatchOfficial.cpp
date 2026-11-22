#include "personnel/MatchOfficial.h"
#include "league/LeagueRuleError.h"
#include <iostream>

MatchOfficial::MatchOfficial(const std::string& id, const std::string& name, int age,
                             const std::string& gender, const std::string& officiatingRole,
                             const std::string& rank, bool available)
    : Person(id, name, age, gender), officiatingRole(officiatingRole), rank(rank),
      available(available) {
    if (officiatingRole.find_first_not_of(" \t\r\n") == std::string::npos ||
        rank.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw LeagueRuleError("A match official needs a role and a rank.");
    }
}

void MatchOfficial::display() const {
    std::cout << getName() << " (Match Official " << getId() << ", " << officiatingRole
              << ", " << rank << (available ? "" : ", unavailable") << ")\n";
}

std::string MatchOfficial::serialize() const {
    return Codec::join({getId(), getName(), std::to_string(getAge()), getGender(),
                        officiatingRole, rank, available ? "1" : "0"});
}

MatchOfficial MatchOfficial::fromRecord(const std::vector<std::string>& f) {
    Codec::requireFields(f, 7, "MatchOfficial");
    return MatchOfficial(f[0], f[1], Codec::toInt(f[2], "age"), f[3], f[4], f[5],
                         Codec::toBool(f[6]));
}
