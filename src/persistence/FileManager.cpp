#include "FileManager.h"
#include <filesystem>
#include <ctime>
#include <iomanip>

namespace fs = std::filesystem;

std::string FileManager::timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    std::ostringstream o; o << std::put_time(&tmv, "%Y-%m-%d %H:%M:%S");
    return o.str();
}

void FileManager::ensureDirectory(const std::string& dir) {
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) throw FileException("Cannot create directory " + dir + ": " + ec.message());
}

bool FileManager::exists(const std::string& path) { return fs::exists(path); }

// Writes to a temp file first, then replaces the real file, so a crash mid-save cannot corrupt data
void FileManager::saveLines(const std::string& path, const std::vector<std::string>& lines, const std::string& label) {
    fs::path p(path);
    if (p.has_parent_path()) ensureDirectory(p.parent_path().string());
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp);
        if (!out) throw FileException("Cannot open file for writing: " + tmp);
        out << "# KPFL data | " << label << " | saved " << timestamp() << "\n";
        for (const auto& l : lines) out << l << "\n";
        out.flush();
        if (!out) throw FileException("Write failed for: " + tmp);
    }
    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) throw FileException("Cannot replace " + path + ": " + ec.message());
}

std::vector<std::string> FileManager::loadLines(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw FileNotFoundException(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;      // skip blanks and header comment
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> FileManager::loadLinesIfExists(const std::string& path) {
    return exists(path) ? loadLines(path) : std::vector<std::string>{};
}

void FileManager::appendLine(const std::string& path, const std::string& line) {
    fs::path p(path);
    if (p.has_parent_path()) ensureDirectory(p.parent_path().string());
    std::ofstream out(path, std::ios::app);
    if (!out) throw FileException("Cannot open file for appending: " + path);
    out << line << "\n";
    if (!out) throw FileException("Append failed for: " + path);
}

void FileManager::backup(const std::string& path) {
    if (!exists(path)) throw FileNotFoundException(path);
    std::error_code ec;
    fs::copy_file(path, path + ".bak", fs::copy_options::overwrite_existing, ec);
    if (ec) throw FileException("Backup failed for " + path + ": " + ec.message());
}

void FileManager::restoreBackup(const std::string& path) {
    if (!exists(path + ".bak")) throw FileNotFoundException(path + ".bak");
    std::error_code ec;
    fs::copy_file(path + ".bak", path, fs::copy_options::overwrite_existing, ec);
    if (ec) throw FileException("Restore failed for " + path + ": " + ec.message());
}

void FileManager::removeFile(const std::string& path) {
    std::error_code ec;
    fs::remove(path, ec);
}
