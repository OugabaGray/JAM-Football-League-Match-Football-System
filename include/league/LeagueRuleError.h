#ifndef LEAGUE_RULE_ERROR_H
#define LEAGUE_RULE_ERROR_H

#include <stdexcept>
#include <string>

// A custom exception for rejected operations. what() gives the message to the driver.
class LeagueRuleError : public std::runtime_error {
public:
    LeagueRuleError(const std::string& message) : std::runtime_error(message) {}
};

#endif
