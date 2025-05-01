#include "benchmark/optionsManager/options-manager.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <limits>

namespace cs
{

optionsManager::optionsManager(optionsParser& parser)
: parser(parser)
{ }

template<typename T>
T optionsManager::getInteger(const std::string& name, T defaultValue, const std::string& typeName) const
{
	if (!parser.isSet(name))
	{
		return defaultValue;
	}

	try
	{
		const std::string& valueStr = parser.getValue(name);
		size_t pos = 0;
		long long value = std::stoll(valueStr, &pos);

		// Check if the entire string was parsed
		if (pos != valueStr.length())
		{
			throw std::invalid_argument("Invalid characters in number");
		}

		// Check bounds
		if (value < std::numeric_limits<T>::min() || value > std::numeric_limits<T>::max())
		{
			throw std::out_of_range("Value out of range for " + typeName);
		}

		return static_cast<T>(value);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error("Invalid " + typeName + " value for option " + name + ": " + parser.getValue(name) + " (" + e.what() + ")");
	}
}

bool optionsManager::getBool(const std::string& name, bool defaultValue) const
{
	if (!parser.isSet(name))
	{
		return defaultValue;
	}
	std::string value = parser.getValue(name);
	if (value.empty())
	{
		return true;
	}

	std::string lowerValue = value;
	std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);

	if (lowerValue == "true" || lowerValue == "1" || lowerValue == "yes")
	{
		return true;
	}
	if (lowerValue == "false" || lowerValue == "0" || lowerValue == "no")
	{
		return false;
	}

	throw std::runtime_error("Invalid boolean value for option " + name + ": " + value);
}

int optionsManager::getInt(const std::string& name, int defaultValue) const
{
	return getInteger<int>(name, defaultValue, "int");
}

unsigned int optionsManager::getUInt(const std::string& name, unsigned int defaultValue) const
{
	return getInteger<unsigned int>(name, defaultValue, "unsigned int");
}

int8_t optionsManager::getInt8(const std::string& name, int8_t defaultValue) const
{
	return getInteger<int8_t>(name, defaultValue, "int8_t");
}

uint8_t optionsManager::getUInt8(const std::string& name, uint8_t defaultValue) const
{
	return getInteger<uint8_t>(name, defaultValue, "uint8_t");
}

int16_t optionsManager::getInt16(const std::string& name, int16_t defaultValue) const
{
	return getInteger<int16_t>(name, defaultValue, "int16_t");
}

uint16_t optionsManager::getUInt16(const std::string& name, uint16_t defaultValue) const
{
	return getInteger<uint16_t>(name, defaultValue, "uint16_t");
}

int32_t optionsManager::getInt32(const std::string& name, int32_t defaultValue) const
{
	return getInteger<int32_t>(name, defaultValue, "int32_t");
}

uint32_t optionsManager::getUInt32(const std::string& name, uint32_t defaultValue) const
{
	return getInteger<uint32_t>(name, defaultValue, "uint32_t");
}

int64_t optionsManager::getInt64(const std::string& name, int64_t defaultValue) const
{
	return getInteger<int64_t>(name, defaultValue, "int64_t");
}

uint64_t optionsManager::getUInt64(const std::string& name, uint64_t defaultValue) const
{
	return getInteger<uint64_t>(name, defaultValue, "uint64_t");
}

double optionsManager::getDouble(const std::string& name, double defaultValue) const
{
	if (!parser.isSet(name))
	{
		return defaultValue;
	}
	try
	{
		return std::stod(parser.getValue(name));
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error("Invalid double value for option " + name + ": " + parser.getValue(name));
	}
}

std::string optionsManager::getString(const std::string& name, const std::string& defaultValue) const
{
	if (!parser.isSet(name))
	{
		return defaultValue;
	}
	return parser.getValue(name);
}

const std::vector<std::string>& optionsManager::getPositionalArgs() const
{
	return parser.getPositionalArgs();
}

bool optionsManager::isSet(const std::string& name) const
{
	return parser.isSet(name);
}

} // namespace cs