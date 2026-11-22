// Persistable.h - contract every saveable class implements, plus field encoding helpers.
// A record is stored as ONE text line:  field1|field2|field3
#pragma once
#include <string>
#include <vector>
#include <sstream>
#include "FileExceptions.h"

class Persistable {
public:
    virtual ~Persistable() = default;
    virtual std::string serialize() const = 0;   // return one line (no newline); use Codec::join
};

namespace Codec {

inline std::string escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '|':  out += "\\p";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            default:   out += c;
        }
    }
    return out;
}

inline std::string unescape(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            switch (n) {
                case 'p':  out += '|';  break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                default:   out += n;
            }
        } else out += s[i];
    }
    return out;
}

inline std::string join(const std::vector<std::string>& fields) {
    std::string line;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i) line += '|';
        line += escape(fields[i]);
    }
    return line;
}

inline std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    for (char c : line) {
        if (c == '|') { fields.push_back(unescape(cur)); cur.clear(); }
        else cur += c;
    }
    fields.push_back(unescape(cur));
    return fields;
}

template <class T> std::string str(const T& v) { std::ostringstream o; o << v; return o.str(); }

// Safe conversions - throw DataFormatException instead of silently returning 0
inline int toInt(const std::string& s, const std::string& field = "value") {
    try {
        size_t pos; int v = std::stoi(s, &pos);
        if (pos != s.size()) throw std::invalid_argument("trailing");
        return v;
    } catch (...) { throw DataFormatException("'" + s + "' is not a valid integer for " + field); }
}
inline double toDouble(const std::string& s, const std::string& field = "value") {
    try {
        size_t pos; double v = std::stod(s, &pos);
        if (pos != s.size()) throw std::invalid_argument("trailing");
        return v;
    } catch (...) { throw DataFormatException("'" + s + "' is not a valid number for " + field); }
}
inline bool toBool(const std::string& s) { return s == "1" || s == "true"; }

// Guard against short/corrupt lines: call at the top of each class's fromRecord()
inline void requireFields(const std::vector<std::string>& f, size_t n, const std::string& what) {
    if (f.size() != n)
        throw DataFormatException(what + " expects " + std::to_string(n) + " fields but found " + std::to_string(f.size()));
}
} // namespace Codec
