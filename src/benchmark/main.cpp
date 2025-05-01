#include <iostream>

#include "benchmark/optionsManager/options-manager.h"

int main(int argc, char* argv[])
{
	cs::optionsParser parser;

	parser.addOption("help", 'h', "Show this help message");
	parser.addOption("verbose", 'v', "Enable verbose output");
	parser.addOption("output", 'o', "Output file name", true);
	parser.addOption("count", 'c', "Number of iterations", true);

	try
	{
		parser.parse(argc, argv);

		cs::optionsManager options(parser);

		if (options.isSet("help"))
		{
			parser.printHelp();
			return 0;
		}

		bool verbose = options.getBool("verbose");
		std::string outputFile = options.getString("output", "default.txt");
		int count = options.getInt("count", 10);

		std::cout << "Verbose: " << verbose << "\n";
		std::cout << "Output file: " << outputFile << "\n";
		std::cout << "Count: " << count << "\n";

		const auto& positional = options.getPositionalArgs();
		if (!positional.empty())
		{
			std::cout << "Positional arguments:\n";
			for (const auto& arg : positional)
			{
				std::cout << "  " << arg << "\n";
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		parser.printHelp();
		return 1;
	}

	return 0;
}