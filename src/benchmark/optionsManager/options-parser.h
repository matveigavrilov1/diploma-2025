#pragma once

#include <string>
#include <vector>
#include <map>

namespace cs
{

class optionsParser
{
public:
	optionsParser() = default;

	void addOption(const std::string& name, char shortName = '\0', const std::string& help = "", bool requiresValue = false);
	void parse(int argc, char* argv[]);
	void printHelp() const;

	const std::vector<std::string>& getPositionalArgs() const;
	bool isSet(const std::string& name) const;
	std::string getValue(const std::string& name) const;

private:
	struct optionInfo
	{
		char shortName;
		std::string help;
		bool requiresValue;
		bool isSet;
		std::string value;
	};

	std::string programName;
	std::map<std::string, optionInfo> options;
	std::map<char, std::string> shortOptions;
	std::vector<std::string> positionalArgs;
};

} // namespace cs