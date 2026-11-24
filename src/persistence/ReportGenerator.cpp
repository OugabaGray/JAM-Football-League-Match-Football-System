#include "ReportGenerator.h"
#include "FileManager.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>

static std::string repeat(char c, size_t n) { return std::string(n, c); }

void ReportGenerator::print(const Report& r, std::ostream& os) {
    std::vector<size_t> w(r.headers.size());
    for (size_t i = 0; i < r.headers.size(); ++i) w[i] = r.headers[i].size();
    for (const auto& row : r.rows)
        for (size_t i = 0; i < row.size() && i < w.size(); ++i) w[i] = std::max(w[i], row[i].size());

    size_t total = 1;
    for (size_t x : w) total += x + 3;

    os << "\n" << repeat('=', total) << "\n  " << r.title << "\n" << repeat('=', total) << "\n";
    os << "|";
    for (size_t i = 0; i < w.size(); ++i) os << " " << std::left << std::setw(static_cast<int>(w[i])) << r.headers[i] << " |";
    os << "\n" << repeat('-', total) << "\n";
    if (r.rows.empty()) os << "  (no records)\n";
    for (const auto& row : r.rows) {
        os << "|";
        for (size_t i = 0; i < w.size(); ++i)
            os << " " << std::left << std::setw(static_cast<int>(w[i])) << (i < row.size() ? row[i] : "") << " |";
        os << "\n";
    }
    os << repeat('-', total) << "\n";
    os << "  Records: " << r.rows.size() << "\n";
    if (!r.footer.empty()) os << "  " << r.footer << "\n";
}

void ReportGenerator::saveAsText(const Report& r, const std::string& path) {
    std::filesystem::path p(path);
    if (p.has_parent_path()) FileManager::ensureDirectory(p.parent_path().string());
    std::ofstream out(path);
    if (!out) throw FileException("Cannot open report file: " + path);
    out << "Generated: " << FileManager::timestamp() << "\n";
    print(r, out);
    if (!out) throw FileException("Write failed for report: " + path);
}

static std::string csvCell(const std::string& s) {
    if (s.find_first_of(",\"\n") == std::string::npos) return s;
    std::string o = "\"";
    for (char c : s) { if (c == '"') o += '"'; o += c; }
    return o + "\"";
}

void ReportGenerator::saveAsCsv(const Report& r, const std::string& path) {
    std::filesystem::path p(path);
    if (p.has_parent_path()) FileManager::ensureDirectory(p.parent_path().string());
    std::ofstream out(path);
    if (!out) throw FileException("Cannot open CSV file: " + path);
    for (size_t i = 0; i < r.headers.size(); ++i) out << (i ? "," : "") << csvCell(r.headers[i]);
    out << "\n";
    for (const auto& row : r.rows) {
        for (size_t i = 0; i < row.size(); ++i) out << (i ? "," : "") << csvCell(row[i]);
        out << "\n";
    }
    if (!out) throw FileException("Write failed for CSV: " + path);
}

std::string ReportGenerator::fixed(double v, int d) {
    std::ostringstream o; o << std::fixed << std::setprecision(d) << v; return o.str();
}

std::string ReportGenerator::money(double amount) {
    std::string s = fixed(amount, 2);
    size_t dot = s.find('.');
    size_t start = (s[0] == '-') ? 1 : 0;
    for (int i = static_cast<int>(dot) - 3; i > static_cast<int>(start); i -= 3) s.insert(i, ",");
    return "$" + s;
}

// Money is stored as integer cents across the project, so no floating-point rounding is involved
std::string ReportGenerator::moneyCents(long long cents, const std::string& symbol) {
    bool neg = cents < 0;
    unsigned long long a = static_cast<unsigned long long>(neg ? -cents : cents);
    std::string whole = std::to_string(a / 100);
    for (int i = static_cast<int>(whole.size()) - 3; i > 0; i -= 3) whole.insert(i, ",");
    std::string frac = std::to_string(a % 100);
    if (frac.size() < 2) frac = "0" + frac;
    return std::string(neg ? "-" : "") + symbol + whole + "." + frac;
}
