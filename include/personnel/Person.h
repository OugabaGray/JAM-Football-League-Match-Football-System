#ifndef PERSON_H
#define PERSON_H

#include <string>

class Person {
private:
    std::string id;
    std::string name;
    int age;
    std::string gender;

protected:
    Person(const std::string& id, const std::string& name, int age,
           const std::string& gender);

public:
    virtual ~Person() = default;
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    int getAge() const { return age; }
    std::string getGender() const { return gender; }

    virtual void display() const = 0;
};

#endif
