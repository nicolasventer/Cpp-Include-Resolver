// Copyright (c) Nicolas VENTER All rights reserved.

#pragma once

#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

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
	std::vector<std::filesystem::path> toParseFolderList;
	// set of folders to include in order to help resolve includes
	std::vector<std::filesystem::path> includeFolderList;
	// list of folders that can be used in order to resolve includes
	std::vector<std::filesystem::path> resolveFolderList;
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
	std::set<std::filesystem::path> invalidPathSet;
	// set of unresolved includes
	std::set<UnresolvedInclude> unresolvedIncludeSet;
	// map of conflicted includes
	// key: text describing the include, value: reference of the include
	std::multimap<std::filesystem::path, ConflictedInclude> conflictedIncludeMap;
	// set of folders to include in order to resolve all includes
	std::set<PrettyPath> resolveIncludeFolderSet;
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

namespace std
{
	template <> struct hash<std::filesystem::path>
	{
		std::size_t operator()(const std::filesystem::path& path) const;
	};
} // namespace std
