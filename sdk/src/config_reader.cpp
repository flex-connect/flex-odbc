#include "flex_sdk/config_reader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace flexodbc {

ConfigReader::ConfigReader(const std::string& filepath) {
    loadProperties(filepath);
}

std::string ConfigReader::getProperty(const std::string& key) const {
    auto it = properties.find(key);
    if (it != properties.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Property not found: " + key);
    }
}

bool ConfigReader::hasProperty(const std::string& key) const {
    return properties.count(key) > 0;
}

void ConfigReader::loadProperties(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open properties file: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            // Trim whitespace from key and value
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            properties[key] = value;
        }
    }
}

} // namespace flexodbc

