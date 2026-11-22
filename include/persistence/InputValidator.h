// InputValidator.h - reusable validation + safe console input. Throws ValidationException.
#pragma once
#include <string>
#include <iostream>
#include <limits>
#include <cctype>
#include "FileExceptions.h"

class InputValidator {
public:
    static std::string requireNonEmpty(const std::string& s, const std::string& field) {
        if (trim(s).empty()) throw ValidationException(field + " cannot be empty");
        return trim(s);
    }
    static int requireInRange(int v, int lo, int hi, const std::string& field) {
        if (v < lo || v > hi)
            throw ValidationException(field + " must be between " + std::to_string(lo) + " and " + std::to_string(hi));
        return v;
    }
    static int requireNonNegative(int v, const std::string& field) {
        if (v < 0) throw ValidationException(field + " cannot be negative");
        return v;
    }
    static double requireNonNegative(double v, const std::string& field) {
        if (v < 0) throw ValidationException(field + " cannot be negative");
        return v;
    }
    static int requirePositive(int v, const std::string& field) {
        if (v <= 0) throw ValidationException(field + " must be greater than zero");
        return v;
    }
    static int parseInt(const std::string& s, const std::string& field) {
        try {
            size_t pos; int v = std::stoi(trim(s), &pos);
            if (pos != trim(s).size()) throw std::invalid_argument("x");
            return v;
        } catch (...) { throw ValidationException(field + " must be a whole number"); }
    }
    static double parseDouble(const std::string& s, const std::string& field) {
        try {
            size_t pos; double v = std::stod(trim(s), &pos);
            if (pos != trim(s).size()) throw std::invalid_argument("x");
            return v;
        } catch (...) { throw ValidationException(field + " must be a number"); }
    }
    // YYYY-MM-DD (checks month length and leap years)
    static std::string requireDate(const std::string& s, const std::string& field = "Date") {
        std::string d = trim(s);
        if (d.size() != 10 || d[4] != '-' || d[7] != '-' || !digits(d, {0,1,2,3,5,6,8,9}))
            throw ValidationException(field + " must use the format YYYY-MM-DD");
        int y = std::stoi(d.substr(0, 4)), m = std::stoi(d.substr(5, 2)), day = std::stoi(d.substr(8, 2));
        static const int dim[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
        if (m < 1 || m > 12) throw ValidationException(field + " has an invalid month");
        int maxDay = dim[m - 1] + ((m == 2 && leap) ? 1 : 0);
        if (day < 1 || day > maxDay) throw ValidationException(field + " has an invalid day");
        return d;
    }
    // HH:MM 24-hour
    static std::string requireTime(const std::string& s, const std::string& field = "Time") {
        std::string t = trim(s);
        if (t.size() != 5 || t[2] != ':' || !digits(t, {0,1,3,4}))
            throw ValidationException(field + " must use the format HH:MM");
        int h = std::stoi(t.substr(0, 2)), m = std::stoi(t.substr(3, 2));
        if (h > 23 || m > 59) throw ValidationException(field + " is not a valid time");
        return t;
    }
    // Letters, spaces, hyphens, apostrophes, periods only
    static std::string requireName(const std::string& s, const std::string& field = "Name") {
        std::string n = requireNonEmpty(s, field);
        for (char c : n)
            if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '-' && c != '\'' && c != '.')
                throw ValidationException(field + " contains invalid characters");
        return n;
    }
    // Minimum-age rule (league sets the minimum; pass it in)
    static int requireMinAge(int age, int minAge, const std::string& who = "Person") {
        if (age < minAge) throw ValidationException(who + " must be at least " + std::to_string(minAge) + " years old");
        return age;
    }

    // ----- Console helpers: loop until valid input -----
    static int readInt(const std::string& prompt, int lo, int hi) {
        while (true) {
            std::cout << prompt;
            std::string line; std::getline(std::cin, line);
            try { return requireInRange(parseInt(line, "Input"), lo, hi, "Input"); }
            catch (const ValidationException& e) { std::cout << "  Invalid: " << e.what() << "\n"; }
        }
    }
    static double readDouble(const std::string& prompt, double minValue = 0.0) {
        while (true) {
            std::cout << prompt;
            std::string line; std::getline(std::cin, line);
            try {
                double v = parseDouble(line, "Input");
                if (v < minValue) throw ValidationException("Input must be at least " + std::to_string(minValue));
                return v;
            } catch (const ValidationException& e) { std::cout << "  Invalid: " << e.what() << "\n"; }
        }
    }
    static std::string readNonEmpty(const std::string& prompt, const std::string& field = "Input") {
        while (true) {
            std::cout << prompt;
            std::string line; std::getline(std::cin, line);
            try { return requireNonEmpty(line, field); }
            catch (const ValidationException& e) { std::cout << "  Invalid: " << e.what() << "\n"; }
        }
    }

    static std::string trim(const std::string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
    }
private:
    static bool digits(const std::string& s, std::initializer_list<int> idx) {
        for (int i : idx) if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
        return true;
    }
};
