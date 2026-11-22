// FileExceptions.h - custom exceptions for file handling, validation and lookup failures
#pragma once
#include <stdexcept>
#include <string>

class FileException : public std::runtime_error {
public:
    explicit FileException(const std::string& msg) : std::runtime_error(msg) {}
};
class FileNotFoundException : public FileException {
public:
    explicit FileNotFoundException(const std::string& path)
        : FileException("File not found or cannot be opened: " + path) {}
};
class DataFormatException : public FileException {   // corrupt / malformed stored data
public:
    explicit DataFormatException(const std::string& msg) : FileException("Data format error: " + msg) {}
};
class RecordNotFoundException : public FileException {
public:
    explicit RecordNotFoundException(const std::string& msg) : FileException("Record not found: " + msg) {}
};
class ValidationException : public std::invalid_argument {
public:
    explicit ValidationException(const std::string& msg) : std::invalid_argument(msg) {}
};
