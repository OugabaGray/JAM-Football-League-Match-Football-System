#ifndef PLAYER_H
#define PLAYER_H

#include "Persistable.h"
#include "personnel/Person.h"
#include <string>
#include <vector>

enum class PlayingStatus { Active, Injured, Suspended, Retired };

std::string playingStatusName(PlayingStatus status);

class Player : public Person, public Persistable {
private:
    int jerseyNumber;
    std::string position;
    int goals;
    PlayingStatus playingStatus;

public:
    static constexpr int MINIMUM_AGE = 16;
    static constexpr int MIN_JERSEY = 1;
    static constexpr int MAX_JERSEY = 99;

    Player(const std::string& id, const std::string& name, int age, const std::string& gender,
           int jerseyNumber, const std::string& position,
           PlayingStatus playingStatus = PlayingStatus::Active, int goals = 0);

    int getJerseyNumber() const { return jerseyNumber; }
    std::string getPosition() const { return position; }
    int getGoals() const { return goals; }
    PlayingStatus getPlayingStatus() const { return playingStatus; }

    void setJerseyNumber(int number);
    void setPosition(const std::string& newPosition);
    void setPlayingStatus(PlayingStatus status) { playingStatus = status; }
    void addGoals(int scored);

    void display() const override;
    std::string serialize() const override;
    static Player fromRecord(const std::vector<std::string>& f);
};

#endif
