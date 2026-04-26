#ifndef FLEX_ODBC_CONFIG_READER_HPP
#define FLEX_ODBC_CONFIG_READER_HPP

#include <string>
#include <map>
#include <stdexcept> // For std::runtime_error

namespace flexodbc {

class ConfigReader {
public:
    explicit ConfigReader(const std::string& filepath);

    std::string getProperty(const std::string& key) const;
    bool hasProperty(const std::string& key) const;

private:
    std::map<std::string, std::string> properties;
    void loadProperties(const std::string& filepath);
};

} // namespace flexodbc

#endif // FLEX_ODBC_CONFIG_READER_HPP
