// FileManager.h - sequential (text) and random access (binary) persistence for the KPFL system
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <fstream>
#include <type_traits>
#include <algorithm>
#include <filesystem>
#include "Persistable.h"

// Central file names so every module uses the same ones
namespace DataFiles {
    const std::string TEAMS        = "data/teams.txt";
    const std::string PLAYERS      = "data/players.txt";
    const std::string COACHES      = "data/coaches.txt";
    const std::string STADIUMS     = "data/stadiums.txt";
    const std::string OFFICIALS    = "data/officials.txt";
    const std::string FIXTURES     = "data/fixtures.txt";
    const std::string RESULTS      = "data/results.txt";
    const std::string PRODUCTS_BIN = "data/products.bin";   // random access
    const std::string TICKETS_BIN  = "data/tickets.bin";    // random access
}

// Outcome of a safe load: tells the caller what happened without throwing
struct LoadResult {
    bool ok = false;               // true when new data was accepted
    bool usedBackup = false;       // true when the .bak copy had to be used
    size_t loaded = 0;             // records accepted
    std::vector<std::string> messages;   // reasons for any rejection / recovery
};

class FileManager {
public:
    // ---------- Sequential access (text files) ----------
    static void saveLines(const std::string& path, const std::vector<std::string>& lines,
                          const std::string& label = "");
    static std::vector<std::string> loadLines(const std::string& path);          // throws if missing
    static std::vector<std::string> loadLinesIfExists(const std::string& path);  // empty on first run
    static void appendLine(const std::string& path, const std::string& line);

    // Save any collection of objects (or pointers/smart pointers) deriving from Persistable
    template <class Container>
    static void saveCollection(const std::string& path, const Container& items, const std::string& label = "") {
        std::vector<std::string> lines;
        for (const auto& item : items) lines.push_back(asPersistable(item).serialize());
        saveLines(path, lines, label);
    }

    // Load objects; 'factory' builds one object from the fields of one line, e.g.
    //   FileManager::loadCollection<Player>(DataFiles::PLAYERS, Player::fromRecord);
    template <class T>
    static std::vector<T> loadCollection(const std::string& path,
                                         const std::function<T(const std::vector<std::string>&)>& factory,
                                         bool missingIsEmpty = true) {
        auto lines = missingIsEmpty ? loadLinesIfExists(path) : loadLines(path);
        std::vector<T> out;
        int lineNo = 0;
        for (const auto& line : lines) {
            ++lineNo;
            try { out.push_back(factory(Codec::split(line))); }
            catch (const DataFormatException& e) {
                std::string m = e.what();
                const std::string pre = "Data format error: ";
                if (m.rfind(pre, 0) == 0) m = m.substr(pre.size());      // avoid a doubled prefix
                throw DataFormatException(path + ", record " + std::to_string(lineNo) + ": " + m);
            }
        }
        return out;
    }

    // ---------- Safe load ----------
    // Loads into a TEMPORARY vector, validates it, and only then replaces 'target'.
    // If the file is missing/corrupt/invalid and a .bak copy exists, the backup is tried next;
    // the bad file is kept as <path>.corrupt and the backup is restored in its place.
    // On total failure 'target' is left exactly as it was (no half-loaded state).
    template <class T>
    static LoadResult safeLoad(std::vector<T>& target, const std::string& path,
                               const std::function<T(const std::vector<std::string>&)>& factory,
                               const std::function<void(const std::vector<T>&)>& validator = nullptr,
                               bool tryBackup = true) {
        LoadResult res;
        auto attempt = [&](const std::string& p, std::vector<T>& out) -> bool {
            try {
                out = loadCollection<T>(p, factory, false);
                if (validator) validator(out);
                return true;
            } catch (const std::exception& e) {
                res.messages.push_back("Rejected " + p + " - " + e.what());
                return false;
            }
        };
        std::vector<T> staged;
        if (attempt(path, staged)) {
            res.ok = true; res.loaded = staged.size();
            target = std::move(staged);
            return res;
        }
        const std::string bak = path + ".bak";
        if (tryBackup && exists(bak) && attempt(bak, staged)) {
            try {
                if (exists(path)) std::filesystem::copy_file(path, path + ".corrupt",
                                       std::filesystem::copy_options::overwrite_existing);
                std::filesystem::copy_file(bak, path, std::filesystem::copy_options::overwrite_existing);
            } catch (const std::exception& e) {
                res.messages.push_back(std::string("Could not restore backup file: ") + e.what());
            }
            res.ok = true; res.usedBackup = true; res.loaded = staged.size();
            res.messages.push_back("Recovered " + path + " from backup (" + std::to_string(staged.size()) + " records)");
            target = std::move(staged);
        }
        return res;
    }

    // ---------- Utilities ----------
    static bool exists(const std::string& path);
    static void ensureDirectory(const std::string& dir);
    static void backup(const std::string& path);              // copies to path.bak
    static void restoreBackup(const std::string& path);
    static void removeFile(const std::string& path);
    static std::string timestamp();

private:
    template <class T> static const Persistable& asPersistable(const T& v) {
        if constexpr (std::is_base_of<Persistable, T>::value) return v;
        else return *v;     // pointer / smart pointer
    }
};

// ---------- Random access (fixed-length binary records) ----------
// Record must be a plain struct with fixed-size members (char arrays, int, double, bool).
// Each record sits at a predictable position: byte offset = index * sizeof(Slot).
// Deleting is "logical": the slot is flagged inactive so other indexes never shift.
template <typename Record>
class RandomAccessFile {
    static_assert(std::is_trivially_copyable<Record>::value,
                  "Record must be trivially copyable (use char arrays, not std::string)");
    struct Slot { bool active; Record data; };

    std::string path_;
    mutable std::fstream f_;

    void open() {
        f_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
        if (!f_) {   // first run: create then reopen for read/write
            std::ofstream create(path_, std::ios::binary);
            if (!create) throw FileNotFoundException(path_);
            create.close();
            f_.clear();
            f_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
            if (!f_) throw FileNotFoundException(path_);
        }
    }
    Slot readSlot(size_t index) const {
        if (index >= slotCount()) throw RecordNotFoundException("index " + std::to_string(index) + " is out of range");
        Slot s{};
        f_.clear();
        f_.seekg(static_cast<std::streamoff>(index * sizeof(Slot)));
        f_.read(reinterpret_cast<char*>(&s), sizeof(Slot));
        if (!f_) throw FileException("Failed reading record " + std::to_string(index) + " from " + path_);
        return s;
    }
    void writeSlot(size_t index, const Slot& s) {
        f_.clear();
        f_.seekp(static_cast<std::streamoff>(index * sizeof(Slot)));
        f_.write(reinterpret_cast<const char*>(&s), sizeof(Slot));
        f_.flush();
        if (!f_) throw FileException("Failed writing record " + std::to_string(index) + " to " + path_);
    }

public:
    explicit RandomAccessFile(const std::string& path) : path_(path) { open(); }
    RandomAccessFile(const RandomAccessFile&) = delete;
    RandomAccessFile& operator=(const RandomAccessFile&) = delete;

    size_t slotCount() const {                         // includes deleted slots
        f_.clear();
        f_.seekg(0, std::ios::end);
        return static_cast<size_t>(f_.tellg()) / sizeof(Slot);
    }
    size_t append(const Record& r) {                   // returns the new record's index
        size_t idx = slotCount();
        Slot s{}; s.active = true; s.data = r;
        writeSlot(idx, s);
        return idx;
    }
    void update(size_t index, const Record& r) {       // overwrite an existing active record
        if (!readSlot(index).active) throw RecordNotFoundException("record " + std::to_string(index) + " was deleted");
        Slot s{}; s.active = true; s.data = r;
        writeSlot(index, s);
    }
    Record read(size_t index) const {
        Slot s = readSlot(index);
        if (!s.active) throw RecordNotFoundException("record " + std::to_string(index) + " was deleted");
        return s.data;
    }
    void remove(size_t index) {
        Slot s = readSlot(index);
        if (!s.active) throw RecordNotFoundException("record " + std::to_string(index) + " already deleted");
        s.active = false;
        writeSlot(index, s);
    }
    bool isActive(size_t index) const { return index < slotCount() && readSlot(index).active; }

    std::vector<Record> readAllActive() const {
        std::vector<Record> out;
        for (size_t i = 0, n = slotCount(); i < n; ++i) {
            Slot s = readSlot(i);
            if (s.active) out.push_back(s.data);
        }
        return out;
    }
    size_t activeCount() const { return readAllActive().size(); }

    // Linear search; returns index or throws RecordNotFoundException
    size_t findFirst(const std::function<bool(const Record&)>& pred) const {
        for (size_t i = 0, n = slotCount(); i < n; ++i) {
            Slot s = readSlot(i);
            if (s.active && pred(s.data)) return i;
        }
        throw RecordNotFoundException("no record matches the search condition");
    }

    // Physically remove deleted slots (NOTE: indexes change afterwards)
    void compact() {
        auto live = readAllActive();
        f_.close();
        std::ofstream trunc(path_, std::ios::binary | std::ios::trunc);
        trunc.close();
        open();
        for (const auto& r : live) append(r);
    }
};

// Helper for filling fixed-size char arrays safely (always null-terminated)
template <size_t N>
void copyToFixed(char (&dest)[N], const std::string& src) {
    std::fill(dest, dest + N, '\0');
    std::copy(src.begin(), src.begin() + std::min(src.size(), N - 1), dest);
}
