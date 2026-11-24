// ReportGenerator.h - formats tabular data as console tables and saves reports to files.
// It never touches other classes: teammates pass in headers + rows of strings.
#pragma once
#include <string>
#include <vector>
#include <ostream>

struct Report {
    std::string title;
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    std::string footer;          // e.g. "Total revenue: $12,500.00"
};

class ReportGenerator {
public:
    static void print(const Report& r, std::ostream& os);          // aligned console table
    static void saveAsText(const Report& r, const std::string& path);
    static void saveAsCsv(const Report& r, const std::string& path); // optional extension: CSV export

    // Formatting helpers for money and numbers
    static std::string money(double amount);          // "$1,234.50"
    static std::string moneyCents(long long cents, const std::string& symbol = "J$"); // integer cents -> "J$1,500.00"
    static std::string fixed(double v, int decimals = 2);
};
