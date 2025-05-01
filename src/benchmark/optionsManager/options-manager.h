#pragma once

#include "benchmark/optionsManager/options-parser.h"
#include <string>

namespace cs
{

class optionsManager
{
public:
	optionsManager(optionsParser& parser);

	bool getBool(const std::string& name, bool defaultValue = false);
	int getInt(const std::string& name, int defaultValue = 0);
	double getDouble(const std::string& name, double defaultValue = 0.0);
	std::string getString(const std::string& name, const std::string& defaultValue = "");

	const std::vector<std::string>& getPositionalArgs() const;
	bool isSet(const std::string& name) const;

private:
	optionsParser& parser;
};

} // namespace cs
