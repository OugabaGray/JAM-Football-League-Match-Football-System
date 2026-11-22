#ifndef PLAYER_H
#define PLAYER_H

#include "Persistable.h"
#include "personnel/Person.h"
#include <iostream>
#include <string>
#include <vector>

enum class PlayingStatus { Active, Injured, Suspended, Retired };

inline std::string playingStatusName(PlayingStatus status) {
    switch (status) {
        case PlayingStatus::Active: return "Active";
        case PlayingStatus::Injured: return "Injured";
        case PlayingStatus::Suspended: return "Suspended";
        default: return "Retired";
    }
}

class Player : public Person, public Persistable {
private:
    int jerseyNumber;
    std::string position;
    int goals;
    PlayingStatus playingStatus;

    static bool blank(const std::string& text) {
        return text.find_first_not_of(" \t\r\n") == std::string::npos;
    }

    static void checkJersey(int number) {
        if (number < MIN_JERSEY || number > MAX_JERSEY) {
            throw LeagueRuleError("A jersey number must be between 1 and 99.");
        }
    }

public:
    static constexpr int MINIMUM_AGE = 16;
    static constexpr int MIN_JERSEY = 1;
    static constexpr int MAX_JERSEY = 99;

    Player(const std::string& id, const std::string& name, int age, const std::string& gender,
           int jerseyNumber, const std::string& position,
           PlayingStatus playingStatus = PlayingStatus::Active, int goals = 0)
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

    int getJerseyNumber() const { return jerseyNumber; }
    std::string getPosition() const { return position; }
    int getGoals() const { return goals; }
    PlayingStatus getPlayingStatus() const { return playingStatus; }

    void setJerseyNumber(int number) {
        checkJersey(number);
        jerseyNumber = number;
    }

    void setPosition(const std::string& newPosition) {
        if (blank(newPosition)) {
            throw LeagueRuleError("A player needs a position.");
        }
        position = newPosition;
    }

    void setPlayingStatus(PlayingStatus status) { playingStatus = status; }

    void addGoals(int scored) {
        if (scored <= 0) {
            throw LeagueRuleError("Goals added must be positive.");
        }
        goals += scored;
    }

    void display() const override {
        std::cout << getName() << " (Player " << getId() << ") | #" << jerseyNumber << " | "
                  << position << " | Age " << getAge() << " | " << getGender() << " | "
                  << goals << " goals | " << playingStatusName(playingStatus) << '\n';
    }

    std::string serialize() const override {
        return Codec::join({getId(), getName(), std::to_string(getAge()), getGender(),
                            std::to_string(jerseyNumber), position, std::to_string(goals),
                            std::to_string(static_cast<int>(playingStatus))});
    }

    static Player fromRecord(const std::vector<std::string>& f) {
        Codec::requireFields(f, 8, "Player");
        int code = Codec::toInt(f[7], "playing status");
        if (code < 0 || code > 3) {
            throw DataFormatException("playing status " + f[7] + " is not 0 to 3");
        }
        return Player(f[0], f[1], Codec::toInt(f[2], "age"), f[3], Codec::toInt(f[4], "jersey"),
                      f[5], static_cast<PlayingStatus>(code), Codec::toInt(f[6], "goals"));
    }
};

#endif
