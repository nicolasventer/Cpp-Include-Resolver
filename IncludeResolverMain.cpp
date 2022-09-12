// Copyright (c) Nicolas VENTER All rights reserved.

#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "IncludeResolver.hpp"

struct ExecutionSettings
{
	// enable the log
	bool bVerbose{false};
	// output file where the result is stored
	std::string outputFile;
};

int usage()
{
	std::cerr << "Usage :" << std::endl;
	std::cerr << "IncludeResolver.exe [Settings...]" << std::endl;
	std::cerr << "  Settings :" << std::endl;
	std::cerr << "    --toParse [toParseFolder...] : list of folders to parse" << std::endl;
	std::cerr << "    --include [includeFolder...] : list of folders used as include" << std::endl;
	std::cerr << "    --resolve [resolveFolder...] : list of folders used to resolve includes" << std::endl;
	std::cerr << "    --output [outputFile] : set the outputFile" << std::endl;
	std::cerr << "  Other :" << std::endl;
	std::cerr << "    --file [filePath...] : append the content of all files as arguments of the command line" << std::endl;
	std::cerr << "    --verbose : enable the log" << std::endl;
	return 1;
}

int getSettings(const std::vector<std::string>& argList,
	IncludeResolverSettings& includeResolverSettings,
	ExecutionSettings& executionSettings)
{
	std::vector<std::string> modifiableArgList = argList;
	for (int i = 1; i < modifiableArgList.size(); ++i)
	{
		std::string arg = modifiableArgList[i];
		if (arg == "--toParse" || arg == "-p")
		{
			++i;
			while (i < modifiableArgList.size() && modifiableArgList[i][0] != '-')
			{
				includeResolverSettings.toParseFolderList.push_back(modifiableArgList[i]);
				++i;
			}
			--i;
		}
		else if (arg == "--include" || arg == "-i")
		{
			++i;
			while (i < modifiableArgList.size() && modifiableArgList[i][0] != '-')
			{
				includeResolverSettings.includeFolderList.push_back(modifiableArgList[i]);
				++i;
			}
			--i;
		}
		else if (arg == "--resolve" || arg == "-r")
		{
			++i;
			while (i < modifiableArgList.size() && modifiableArgList[i][0] != '-')
			{
				includeResolverSettings.resolveFolderList.push_back(modifiableArgList[i]);
				++i;
			}
			--i;
		}
		else if (arg == "--verbose" || arg == "-v")
		{
			executionSettings.bVerbose = true;
		}
		else if (arg == "--output" || arg == "-o")
		{
			++i;
			if (i >= modifiableArgList.size() || modifiableArgList[i][0] == '-')
			{
				std::cerr << "Error: require output file after '--output' argument" << std::endl;
				return usage();
			}
			executionSettings.outputFile = modifiableArgList[i];
		}
		else if (arg == "--file" || arg == "-f")
		{
			++i;
			while (i < modifiableArgList.size() && modifiableArgList[i][0] != '-')
			{
				std::ifstream ifs(modifiableArgList[i]);
				if (!ifs.is_open())
				{
					std::cerr << "Error: can't open file '" << modifiableArgList[i] << "'" << std::endl;
					return usage();
				}
				std::string word;
				std::string stackedWord;
				bool bPushingWord = true;
				while (ifs >> word)
				{
					auto quotePos = word.find('"');
					if (quotePos != std::string::npos && quotePos != 0 && quotePos != word.size() - 1)
					{
						std::cerr << "Error : invalid quote in file '" << modifiableArgList[i] << "' for word '" << word << "'"
								  << std::endl;
						return usage();
					}
					stackedWord += stackedWord.empty() ? word : " " + word;
					if (word[0] == '"') bPushingWord = false;
					if (word[word.size() - 1] == '"') bPushingWord = true;
					if (bPushingWord)
					{
						stackedWord.erase(std::remove(stackedWord.begin(), stackedWord.end(), '"'), stackedWord.end());
						modifiableArgList.push_back(stackedWord);
						stackedWord.clear();
					}
				}
				++i;
			}
			--i;
		}
		else if (arg == "--help" || arg == "-h")
		{
			return usage();
		}
		else
		{
			std::cerr << "Unknown argument : '" << arg << "'" << std::endl;
			return usage();
		}
	}

	return 0;
}

int main(int argc, const char* argv[])
{
	std::vector<std::string> argList;
	for (int i = 0; i < argc; ++i) argList.push_back(argv[i]);
	IncludeResolverSettings includeResolverSettings;
	ExecutionSettings executionSettings;
	if (getSettings(argList, includeResolverSettings, executionSettings)) return 1;

	std::ostream* os = &std::cout;
	std::ofstream ofs;
	if (!executionSettings.outputFile.empty())
	{
		ofs.open(executionSettings.outputFile);
		if (ofs.is_open()) os = &ofs;
	}

	IncludeResolverResult result=
	executionSettings.bVerbose ? [&includeResolverSettings, os]()
	{
		// bVerbose=true
		IncludeResolverResult ret;
		std::cout << "start parsing..." << std::endl;
		auto callback = [](size_t current, size_t total, const std::filesystem::path&)
		{ std::cout << "\r[" << current << "/" << total << "]" << std::flush; };
		ret = include_resolver::computeIncludeResolve(includeResolverSettings, callback);
		std::cout << '\n';
		return ret;
	}()
	:[&includeResolverSettings]()
	{
		// bVerbose=false
		IncludeResolverResult ret;
		auto callback = [](size_t, size_t, const std::filesystem::path&) {};
		ret = include_resolver::computeIncludeResolve(includeResolverSettings, callback);
		return ret;
	}();

	for (const auto& invalidPath : result.invalidPathSet) *os << "'" << invalidPath << "' does not exist" << std::endl;

	*os << '\n';
	if (result.unresolvedIncludeSet.empty()) *os << "No unresolved include" << std::endl;
	else
	{
		*os << result.unresolvedIncludeSet.size() << " unresolved includes:" << std::endl;
		for (const auto& unfoundInclude : result.unresolvedIncludeSet) *os << unfoundInclude << std::endl;
	}
	*os << '\n';
	if (result.conflictedIncludeMap.empty()) *os << "No conflicted include" << std::endl;
	else
	{
		*os << result.conflictedIncludeMap.size() << " conflicted includes:" << std::endl;
		for (const auto& [include, conflictedInclude] : result.conflictedIncludeMap)
			*os << include.string() << ":\n" << conflictedInclude << std::endl;
	}
	*os << '\n';
	if (result.resolveIncludeFolderSet.empty()) *os << "No folders to include" << std::endl;
	else
	{
		*os << result.resolveIncludeFolderSet.size() << " folders to include:" << std::endl;
		for (const auto& projectInclude : result.resolveIncludeFolderSet) *os << projectInclude << std::endl;
	}

	return 0;
}