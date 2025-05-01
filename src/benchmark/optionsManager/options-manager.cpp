#include "benchmark/optionsManager/options-manager.h"

#include <algorithm>
#include <stdexcept>

namespace cs
{

optionsManager::optionsManager(optionsParser& parser)
: parser(parser)
{ }

bool optionsManager::getBool(const std::string& name, bool defaultValue)
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

int optionsManager::getInt(const std::string& name, int defaultValue)
{
	if (!parser.isSet(name))
	{
		return defaultValue;
	}
	try
	{
		return std::stoi(parser.getValue(name));
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error("Invalid integer value for option " + name + ": " + parser.getValue(name));
	}
}

double optionsManager::getDouble(const std::string& name, double defaultValue)
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

std::string optionsManager::getString(const std::string& name, const std::string& defaultValue)
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