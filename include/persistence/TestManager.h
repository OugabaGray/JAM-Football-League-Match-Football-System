// TestManager.h - lightweight test harness that records Normal / Boundary / Invalid cases
// and saves the results as evidence for the testing report.
#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include "FileManager.h"

enum class TestCategory { Normal, Boundary, Invalid };

class TestManager {
    struct Result { std::string name; TestCategory cat; bool passed; std::string detail; };
    std::vector<Result> results_;
    std::string suite_;

    static const char* catName(TestCategory c) {
        switch (c) { case TestCategory::Normal: return "Normal";
                     case TestCategory::Boundary: return "Boundary"; default: return "Invalid"; }
    }
public:
    explicit TestManager(std::string suite = "KPFL Tests") : suite_(std::move(suite)) {}

    void check(const std::string& name, TestCategory cat, bool condition, const std::string& detail = "") {
        results_.push_back({name, cat, condition, detail});
    }
    template <class A, class B>
    void expectEqual(const std::string& name, TestCategory cat, const A& expected, const B& actual) {
        bool ok = (expected == actual);
        std::ostringstream d; d << "expected [" << expected << "] actual [" << actual << "]";
        check(name, cat, ok, d.str());
    }
    // Passes only if f() throws ExceptionType (use for Invalid cases)
    template <class ExceptionType, class F>
    void expectThrows(const std::string& name, TestCategory cat, F f) {
        try { f(); check(name, cat, false, "no exception thrown"); }
        catch (const ExceptionType& e) { check(name, cat, true, std::string("threw: ") + e.what()); }
        catch (const std::exception& e) { check(name, cat, false, std::string("wrong exception: ") + e.what()); }
    }
    // Passes only if f() completes without throwing (use for Normal/Boundary cases)
    template <class F>
    void expectNoThrow(const std::string& name, TestCategory cat, F f) {
        try { f(); check(name, cat, true, "completed"); }
        catch (const std::exception& e) { check(name, cat, false, std::string("unexpected: ") + e.what()); }
    }

    int passedCount() const { int n = 0; for (auto& r : results_) n += r.passed; return n; }
    int failedCount() const { return static_cast<int>(results_.size()) - passedCount(); }

    void print(std::ostream& os) const {
        os << "\n=== " << suite_ << " ===\n";
        int i = 0;
        for (const auto& r : results_)
            os << std::setw(3) << ++i << ". [" << (r.passed ? "PASS" : "FAIL") << "] ["
               << catName(r.cat) << "] " << r.name << " - " << r.detail << "\n";
        os << "Total: " << results_.size() << "  Passed: " << passedCount() << "  Failed: " << failedCount() << "\n";
    }
    void saveReport(const std::string& path) const {
        std::filesystem::path p(path);
        if (p.has_parent_path()) FileManager::ensureDirectory(p.parent_path().string());
        std::ofstream out(path);
        if (!out) throw FileException("Cannot open test report: " + path);
        out << "Generated: " << FileManager::timestamp() << "\n";
        print(out);
    }
};
