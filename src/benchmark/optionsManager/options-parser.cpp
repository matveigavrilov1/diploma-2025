#include "benchmark/optionsManager/options-parser.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

namespace cs
{

void optionsParser::addOption(const std::string& name, char shortName, const std::string& help, bool requiresValue)
{
	optionInfo info;
	info.shortName = shortName;
	info.help = help;
	info.requiresValue = requiresValue;
	info.isSet = false;
	options[name] = info;

	if (shortName != '\0')
	{
		shortOptions[shortName] = name;
	}
}

void optionsParser::parse(int argc, char* argv[])
{
	if (argc > 0)
	{
		programName = argv[0];
	}

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];

		if (arg == "--help")
		{
			printHelp();
			exit(0);
		}

		if (arg.substr(0, 2) == "--")
		{
			std::string optionName = arg.substr(2);
			auto it = options.find(optionName);
			if (it == options.end())
			{
				throw std::runtime_error("Unknown option: " + arg);
			}

			if (it->second.requiresValue)
			{
				if (i + 1 >= argc)
				{
					throw std::runtime_error("Option --" + optionName + " requires a value");
				}
				it->second.value = argv[++i];
			}
			it->second.isSet = true;
		}
		else if (arg[0] == '-')
		{
			char shortName = arg[1];
			auto shortIt = shortOptions.find(shortName);
			if (shortIt == shortOptions.end())
			{
				throw std::runtime_error("Unknown option: " + arg);
			}

			auto& option = options[shortIt->second];
			if (option.requiresValue)
			{
				if (arg.length() > 2)
				{
					option.value = arg.substr(2);
				}
				else
				{
					if (i + 1 >= argc)
					{
						throw std::runtime_error("Option -" + std::string(1, shortName) + " requires a value");
					}
					option.value = argv[++i];
				}
			}
			option.isSet = true;
		}
		else
		{
			positionalArgs.push_back(arg);
		}
	}
}

void optionsParser::printHelp() const
{
	std::cout << "Usage: " << programName << " [options]\n";
	std::cout << "Options:\n";

	for (const auto& pair : options)
	{
		std::cout << "  ";
		if (pair.second.shortName != '\0')
		{
			std::cout << "-" << pair.second.shortName << ", ";
		}
		else
		{
			std::cout << "    ";
		}
		std::cout << "--" << pair.first;

		if (pair.second.requiresValue)
		{
			std::cout << " <value>";
		}

		if (!pair.second.help.empty())
		{
			std::cout << " : " << pair.second.help;
		}

		std::cout << "\n";
	}
}

const std::vector<std::string>& optionsParser::getPositionalArgs() const
{
	return positionalArgs;
}

bool optionsParser::isSet(const std::string& name) const
{
	auto it = options.find(name);
	if (it == options.end())
	{
		throw std::runtime_error("Unknown option: " + name);
	}
	return it->second.isSet;
}

std::string optionsParser::getValue(const std::string& name) const
{
	auto it = options.find(name);
	if (it == options.end())
	{
		throw std::runtime_error("Unknown option: " + name);
	}
	return it->second.value;
}

} // namespace cs