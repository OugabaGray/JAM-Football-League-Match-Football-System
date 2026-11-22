#include "personnel/Player.h"
#include "league/LeagueRuleError.h"
#include <iostream>

namespace {
bool blank(const std::string& text) {
    return text.find_first_not_of(" \t\r\n") == std::string::npos;
}

void checkJersey(int number) {
    if (number < Player::MIN_JERSEY || number > Player::MAX_JERSEY) {
        throw LeagueRuleError("A jersey number must be between 1 and 99.");
    }
}
}

std::string playingStatusName(PlayingStatus status) {
    switch (status) {
        case PlayingStatus::Active: return "Active";
        case PlayingStatus::Injured: return "Injured";
        case PlayingStatus::Suspended: return "Suspended";
        default: return "Retired";
    }
}

Player::Player(const std::string& id, const std::string& name, int age,
               const std::string& gender, int jerseyNumber, const std::string& position,
               PlayingStatus playingStatus, int goals)
    : Person(id, name, age, gender), jerseyNumber(jerseyNumber), position(position),
      goals(goals), playingStatus(playingStatus) {
    if (age < MINIMUM_AGE) {
        throw LeagueRuleError("A player must be at least " + std::to_string(MINIMUM_AGE) +
                              " years old.");
    }
    checkJersey(jerseyNumber);
    if (blank(position)) {
        throw LeagueRuleError("A player needs a position.");
    }
    if (goals < 0) {
        throw LeagueRuleError("Goals cannot be negative.");
    }
}

void Player::setJerseyNumber(int number) {
    checkJersey(number);
    jerseyNumber = number;
}

void Player::setPosition(const std::string& newPosition) {
    if (blank(newPosition)) {
        throw LeagueRuleError("A player needs a position.");
    }
    position = newPosition;
}

void Player::addGoals(int scored) {
    if (scored <= 0) {
        throw LeagueRuleError("Goals added must be positive.");
    }
    goals += scored;
}

void Player::display() const {
    std::cout << getName() << " (Player " << getId() << ") | #" << jerseyNumber << " | "
              << position << " | Age " << getAge() << " | " << getGender() << " | "
              << goals << " goals | " << playingStatusName(playingStatus) << '\n';
}

std::string Player::serialize() const {
    return Codec::join({getId(), getName(), std::to_string(getAge()), getGender(),
                        std::to_string(jerseyNumber), position, std::to_string(goals),
                        std::to_string(static_cast<int>(playingStatus))});
}

Player Player::fromRecord(const std::vector<std::string>& f) {
    Codec::requireFields(f, 8, "Player");
    int code = Codec::toInt(f[7], "playing status");
    if (code < 0 || code > 3) {
        throw DataFormatException("playing status " + f[7] + " is not 0 to 3");
    }
    return Player(f[0], f[1], Codec::toInt(f[2], "age"), f[3], Codec::toInt(f[4], "jersey"),
                  f[5], static_cast<PlayingStatus>(code), Codec::toInt(f[6], "goals"));
}
