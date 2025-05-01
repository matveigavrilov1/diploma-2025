#pragma once

#include "benchmark/optionsManager/options-parser.h"
#include <cstdint>
#include <string>

namespace cs
{

class optionsManager
{
public:
    optionsManager(optionsParser& parser);

    bool getBool(const std::string& name, bool defaultValue = false) const;
    
    // Integer types
    int getInt(const std::string& name, int defaultValue = 0) const;
    unsigned int getUInt(const std::string& name, unsigned int defaultValue = 0) const;
    int8_t getInt8(const std::string& name, int8_t defaultValue = 0) const;
    uint8_t getUInt8(const std::string& name, uint8_t defaultValue = 0) const;
    int16_t getInt16(const std::string& name, int16_t defaultValue = 0) const;
    uint16_t getUInt16(const std::string& name, uint16_t defaultValue = 0) const;
    int32_t getInt32(const std::string& name, int32_t defaultValue = 0) const;
    uint32_t getUInt32(const std::string& name, uint32_t defaultValue = 0) const;
    int64_t getInt64(const std::string& name, int64_t defaultValue = 0) const;
    uint64_t getUInt64(const std::string& name, uint64_t defaultValue = 0) const;
    
    double getDouble(const std::string& name, double defaultValue = 0.0) const;
    std::string getString(const std::string& name, const std::string& defaultValue = "") const;

    const std::vector<std::string>& getPositionalArgs() const;
    bool isSet(const std::string& name) const;

private:
    optionsParser& parser;
    
    template<typename T>
    T getInteger(const std::string& name, T defaultValue, const std::string& typeName) const;
};

} // namespace cs