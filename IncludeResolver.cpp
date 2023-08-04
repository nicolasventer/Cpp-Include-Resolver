// Copyright (c) Nicolas VENTER All rights reserved.

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/*
Usage:
IncludeResolver.exe [Settings...]
  Settings:
	--toParse/-p [toParseFolder...] : list of folders to parse
	--include/-i [includeFolder...] : list of folders used as include
	--resolve/-r [resolveFolder...] : list of folders used to resolve includes
	--output/-o [outputFile] : set the outputFile
  Other:
	--file/-f [filePath...] : append the content of all files as arguments of the command line
	--verbose/-v : enable the log
	--help/-h : display the help
	--help-result/-hr : display the yaml format of the result

ResolverResult yaml:
{
	"invalidPaths": string[],
	"unresolvedIncludes": Object{ ["filePath": string]: string },
	"conflictedIncludes": Object{ ["include": string]: { "includedBy": string[], "canBeResolvedBy": string[] } },
	"resolveIncludeFolders": string[]
}
*/

// same as main
extern "C" __declspec(dllexport) int include_resolver_main(int argc, const char* argv[]);

// small wrapper class for pretty display of path with slash instead of backslash
class PrettyPath : public std::filesystem::path
{
	using parent_type = std::filesystem::path;
	using parent_type::parent_type;

public:
	std::string prettyString() const;
	friend std::ostream& operator<<(std::ostream& os, const PrettyPath& prettyPath);
};

struct IncludeResolverSettings
{
	// list of folders to parse
	std::vector<PrettyPath> toParseFolderList;
	// set of folders to include in order to help resolve includes
	std::vector<PrettyPath> includeFolderList;
	// list of folders that can be used in order to resolve includes
	std::vector<PrettyPath> resolveFolderList;
};

struct IncludeLocation
{
	// file where include was unresolved
	PrettyPath filePath;
	// line of the include
	uint32_t line;

	friend bool operator<(const IncludeLocation& lhs, const IncludeLocation& rhs);

	// can be used to display include location
	friend std::ostream& operator<<(std::ostream& os, const IncludeLocation& unresolvedInclude);
};

// include that have not been resolved
struct UnresolvedInclude : public IncludeLocation
{
	// text describing the include
	std::string include;

	friend bool operator<(const UnresolvedInclude& lhs, const UnresolvedInclude& rhs);

	// can be used to display unresolved include
	friend std::ostream& operator<<(std::ostream& os, const UnresolvedInclude& unresolvedInclude);
};

// include that can be resolve in several ways
struct ConflictedInclude
{
	// set of locations where the include is done
	std::set<IncludeLocation> includeLocationSet;

	// set of folders that can all resolve the include
	std::set<PrettyPath> resolveIncludeFolderSet;

	// can be used to display conflicted include
	friend std::ostream& operator<<(std::ostream& os, const ConflictedInclude& conflictedInclude);
};

struct IncludeResolverResult
{
	// set of invalid paths from Settings
	std::set<PrettyPath> invalidPathSet;
	// set of unresolved includes
	std::set<UnresolvedInclude> unresolvedIncludeSet;
	// map of conflicted includes
	// key: text describing the include, value: reference of the include
	std::map<PrettyPath, ConflictedInclude> conflictedIncludeMap;
	// set of folders to include in order to resolve all includes
	std::set<PrettyPath> resolveIncludeFolderSet;

	// can be used to display the result
	friend std::ostream& operator<<(std::ostream& os, const IncludeResolverResult& includeResolverResult);
};

#define PARSE_STATUS_PARAM size_t current, size_t total, const PrettyPath &filePath
// parameters are captures
#define PARSE_STATUS_LAMBDA(...) [__VA_ARGS__](PARSE_STATUS_PARAM)
using ParseStatusCallback = std::function<void(PARSE_STATUS_PARAM)>;

namespace include_resolver
{
	// compute all folders to include in order to resolve all includes
	IncludeResolverResult computeIncludeResolve(
		const IncludeResolverSettings& settings,
		ParseStatusCallback parseStatusCallback = [](size_t, size_t, const PrettyPath&) {});

	namespace example
	{
		// example of parse status callback that displays the parse status
		void displayParseStatus(PARSE_STATUS_PARAM);
	} // namespace example
} // namespace include_resolver

std::string PrettyPath::prettyString() const
{
	std::string s = string();
	std::replace(s.begin(), s.end(), '\\', '/');
	return s;
}

std::ostream& operator<<(std::ostream& os, const PrettyPath& prettyPath) { return os << prettyPath.prettyString(); }

bool operator<(const IncludeLocation& lhs, const IncludeLocation& rhs)
{
	return lhs.filePath != rhs.filePath ? lhs.filePath < rhs.filePath : lhs.line < rhs.line;
}

// can be used to display include location
std::ostream& operator<<(std::ostream& os, const IncludeLocation& includeLocation)
{
	return os << includeLocation.filePath << ":" << includeLocation.line;
}

bool operator<(const UnresolvedInclude& lhs, const UnresolvedInclude& rhs)
{
	return static_cast<IncludeLocation>(lhs) < static_cast<IncludeLocation>(rhs);
}
std::ostream& operator<<(std::ostream& os, const UnresolvedInclude& unresolvedInclude)
{
	return os << static_cast<IncludeLocation>(unresolvedInclude) << " : " << unresolvedInclude.include;
}

static const char* const TAB = "    ";

std::ostream& operator<<(std::ostream& os, const ConflictedInclude& conflictedInclude)
{
	os << TAB << TAB << "includedBy:";
	for (const auto& includeLocation : conflictedInclude.includeLocationSet)
		os << "\n" << TAB << TAB << TAB << "- " << includeLocation;
	os << "\n" << TAB << TAB << "canBeResolvedBy:";
	for (const auto& resolveIncludeFolder : conflictedInclude.resolveIncludeFolderSet)
		os << "\n" << TAB << TAB << TAB << "- " << resolveIncludeFolder;
	return os;
}

std::ostream& operator<<(std::ostream& os, const IncludeResolverResult& includeResolverResult)
{
	os << "invalidPaths:";
	for (const auto& invalidPath : includeResolverResult.invalidPathSet) os << "\n" << TAB << "- " << invalidPath;
	os << "\nunresolvedIncludes:";
	for (const auto& unresolvedInclude : includeResolverResult.unresolvedIncludeSet) os << "\n" << TAB << unresolvedInclude;
	os << "\nconflictedIncludes:";
	for (const auto& conflictedInclude : includeResolverResult.conflictedIncludeMap)
		os << "\n" << TAB << conflictedInclude.first << ":\n" << conflictedInclude.second;
	os << "\nresolveIncludeFolders:";
	for (const auto& resolveIncludeFolder : includeResolverResult.resolveIncludeFolderSet)
		os << "\n" << TAB << "- " << resolveIncludeFolder;
	return os;
}

namespace include_resolver
{
	namespace utility
	{
		static bool bStartWith(const std::string_view& str, const std::string_view& prefix, std::string_view& subStr)
		{
			if (str.size() < prefix.size()) return false;
			bool result = str.substr(0, prefix.size()) == prefix;
			if (result) subStr = str.substr(prefix.size());
			return result;
		}

		static bool bEndWith(const std::string_view& str, const std::string_view& suffix, std::string_view& subStr)
		{
			if (str.size() < suffix.size()) return false;
			auto offset = str.size() - suffix.size();
			bool result = str.substr(offset) == suffix;
			if (result) subStr = str.substr(0, offset);
			return result;
		}

		static bool isCppFile(const std::string& filePath)
		{
			static auto cppExtensionList = {".h", ".hpp", ".hxx", ".hh", ".c", ".cpp", ".cxx"};
			static std::string_view subStr;
			for (auto& cppExtension : cppExtensionList)
				if (bEndWith(filePath, cppExtension, subStr)) return true;
			return false;
		}

		static void updateCppFileList(const std::filesystem::path& folderPath, std::vector<std::filesystem::path>& cppFileList)
		{
			for (const auto& file : std::filesystem::directory_iterator(folderPath))
			{
				if (file.is_directory())
				{
					updateCppFileList(file.path(), cppFileList);
				}
				else
				{
					if (isCppFile(file.path().string()))
						cppFileList.push_back(std::filesystem::canonical(std::filesystem::absolute(file.path())));
				}
			}
		}
	} // namespace utility

	IncludeResolverResult computeIncludeResolve(const IncludeResolverSettings& settings, ParseStatusCallback parseStatusCallback)
	{
		using namespace utility;

		IncludeResolverResult result;

		std::vector<std::filesystem::path> cppFileToParseList;
		for (const auto& toParseFolder : settings.toParseFolderList) updateCppFileList(toParseFolder, cppFileToParseList);

		for (const auto& includeFolder : settings.includeFolderList)
		{
			if (std::filesystem::exists(includeFolder))
				result.resolveIncludeFolderSet.insert(
					PrettyPath(std::filesystem::canonical(std::filesystem::absolute(includeFolder))));
			else
				result.invalidPathSet.insert(includeFolder);
		}
		std::vector<std::filesystem::path> resolveFileList;
		for (const auto& resolveFolder : settings.resolveFolderList)
		{
			std::filesystem::path resolveFolderPath = resolveFolder;
			if (std::filesystem::exists(resolveFolderPath)) updateCppFileList(resolveFolder, resolveFileList);
			else
				result.invalidPathSet.insert(resolveFolder);
		}
		std::unordered_multimap<std::string, std::filesystem::path> resolveFileMultiMap;
		for (const auto& resolveFile : resolveFileList) resolveFileMultiMap.emplace(resolveFile.filename().string(), resolveFile);

		std::unordered_set<std::filesystem::path> cppFileToParseSet;
		for (const auto& cppFileToParse : cppFileToParseList) cppFileToParseSet.insert(cppFileToParse);

		for (size_t i = 0; i < cppFileToParseList.size(); ++i)
		{
			// copy done since cppFileToParseList can be reallocate at any time
			PrettyPath cppFileToParse = PrettyPath(cppFileToParseList[i]);
			const PrettyPath& filePath = cppFileToParse;

			parseStatusCallback((i + 1), cppFileToParseList.size(), cppFileToParse);

			std::ifstream ifs(static_cast<std::filesystem::path>(cppFileToParse));
			uint32_t lineIndex = 1;
			for (std::string line; std::getline(ifs, line); ++lineIndex)
			{
				std::string_view substr;
				if (!bStartWith(line, "#include ", substr)) continue;
				auto startPos = substr.find_first_of("\"<");
				if (startPos == std::string::npos) continue;
				auto endPos = substr.find_first_of("\">", startPos + 1);
				std::string include = static_cast<std::string>(substr.substr(startPos + 1, endPos - startPos - 1));

				const std::filesystem::path& includePath = cppFileToParse.parent_path() / std::filesystem::path(include);

				static auto addToParse = [&cppFileToParseSet, &cppFileToParseList](const std::filesystem::path& includePath)
				{
					std::filesystem::path canonicalPath = std::filesystem::canonical(std::filesystem::path(includePath));
					if (!cppFileToParseSet.count(canonicalPath))
					{
						cppFileToParseSet.insert(canonicalPath);
						cppFileToParseList.push_back(canonicalPath);
					}
				};

				if (std::filesystem::exists(includePath))
				{
					addToParse(includePath);
					continue;
				}

				static auto findProjectInclude = [&result](const std::string& include, std::filesystem::path& fileFound)
				{
					for (auto& resolveIncludeFolder : result.resolveIncludeFolderSet)
					{
						fileFound = resolveIncludeFolder / std::filesystem::path(include);
						if (std::filesystem::exists(fileFound)) return true;
					}
					return false;
				};

				static auto findInclude = [&resolveFileMultiMap](const std::string& include, ConflictedInclude& conflictedInclude)
				{
					std::string_view substr;
					std::string includeBaseName = std::filesystem::path(include).filename().string();
					auto [it, itEnd] = resolveFileMultiMap.equal_range(includeBaseName);
					for (; it != itEnd; ++it)
					{
						std::string s = it->second.string();
						std::replace(s.begin(), s.end(), '\\', '/');
						if (bEndWith(s, "/" + include, substr))
							conflictedInclude.resolveIncludeFolderSet.insert(PrettyPath(substr));
					}
					return !conflictedInclude.resolveIncludeFolderSet.empty();
				};

				ConflictedInclude conflictedInclude;
				auto it = result.conflictedIncludeMap.find(include);
				if (it != result.conflictedIncludeMap.end())
				{
					IncludeLocation includeLocation;
					includeLocation.filePath = filePath;
					includeLocation.line = lineIndex;
					it->second.includeLocationSet.insert(includeLocation);
					continue;
				}
				if (findInclude(include, conflictedInclude))
				{
					if (conflictedInclude.resolveIncludeFolderSet.size() == 1)
					{
						const PrettyPath& resolveIncludeFolderFound = *conflictedInclude.resolveIncludeFolderSet.begin();
						result.resolveIncludeFolderSet.insert(resolveIncludeFolderFound);
						addToParse(resolveIncludeFolderFound / std::filesystem::path(include));
					}
					else
					{
						IncludeLocation includeLocation;
						includeLocation.filePath = filePath;
						includeLocation.line = lineIndex;
						conflictedInclude.includeLocationSet.insert(includeLocation);
						result.conflictedIncludeMap.emplace(include, conflictedInclude);
						for (const auto& resolveIncludeFolder : conflictedInclude.resolveIncludeFolderSet)
						{
							addToParse(resolveIncludeFolder / std::filesystem::path(include));
						}
					}
					continue;
				}

				std::filesystem::path fileFound;
				if (findProjectInclude(include, fileFound))
				{
					addToParse(fileFound);
					continue;
				}

				UnresolvedInclude unresolvedInclude;
				unresolvedInclude.filePath = filePath;
				unresolvedInclude.include = include;
				unresolvedInclude.line = lineIndex;
				result.unresolvedIncludeSet.insert(unresolvedInclude);
			}
		}
		return result;
	}

	namespace example
	{
		void displayParseStatus(PARSE_STATUS_PARAM)
		{
			std::cout << "[" << current << "/" << total << "] " << filePath << std::endl;
		}
	} // namespace example
} // namespace include_resolver

struct ExecutionSettings
{
	// enable the log
	bool bVerbose{false};
	// output file where the result is stored
	std::string outputFile;
};

int usage(const std::string& argv0)
{
	std::cerr << "Usage:" << std::endl;
	std::cerr << argv0 << " [Settings...]" << std::endl;
	std::cerr << "  Settings:" << std::endl;
	std::cerr << "    --toParse/-p [toParseFolder...] : list of folders to parse" << std::endl;
	std::cerr << "    --include/-i [includeFolder...] : list of folders used as include" << std::endl;
	std::cerr << "    --resolve/-r [resolveFolder...] : list of folders used to resolve includes" << std::endl;
	std::cerr << "    --output/-o [outputFile] : set the outputFile" << std::endl;
	std::cerr << "  Other:" << std::endl;
	std::cerr << "    --file/-f [filePath...] : append the content of all files as arguments of the command line" << std::endl;
	std::cerr << "    --verbose/-v : enable the log" << std::endl;
	std::cerr << "    --help/-h : display the help" << std::endl;
	std::cerr << "    --help-result/-hr : display the yaml format of the result" << std::endl;
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
				return usage(argList[0]);
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
					return usage(argList[0]);
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
						return usage(argList[0]);
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
			return usage(argList[0]);
		}
		else if (arg == "--help-result" || arg == "-hr")
		{
			std::cout << "Result yaml format:" << std::endl;
			std::cout << "  invalidPaths: string[]" << std::endl;
			std::cout << "  unresolvedIncludes: { \"filePath\": string, \"line\": number, \"include\": string }[]" << std::endl;
			std::cout
				<< "  conflictedIncludes: { \"include\": string, \"includedBy\": { \"filePath\": string, \"line\": number }[], "
				   "\"canBeResolvedBy\": string[] }[]"
				<< std::endl;
			std::cout << "  resolveIncludeFolders: string[]" << std::endl;
			return 1;
		}
		else
		{
			std::cerr << "Error: unknown argument '" << arg << "'" << std::endl;
			return usage(argList[0]);
		}
	}

	return 0;
}

int include_resolver_main(int argc, const char* argv[])
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

	*os << result << std::endl;

	return 0;
}

#ifndef NO_MAIN
int main(int argc, const char* argv[]) { return include_resolver_main(argc, argv); }
#endif
