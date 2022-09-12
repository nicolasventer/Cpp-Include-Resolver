# Include Resolver

# Description

IncludeResolver is a **single header c++ library** that allows you to **compute all folders to add to `includePath`** in order to resolve all includes.

IncludeResolver can also be used as an **executable configurable via the command line arguments**.

**Explanation `includePath` by example:**  
[example/src/A.cpp](example/src/A.cpp) contains	 `#include "A.h"`.  
In order to find the file [example/incl/A.h](example/incl/A.h), g++ need to but executed with the argument `-Iexample/incl`.  
So the Include Resolver will prompt the folder `example/incl`.

# Features

- retrieve the folders to include
- detect the conflicts
- store unresolved includes
- write the output result in a file *(main)*

# Installation

Include the file [`IncludeResolver.hpp`](IncludeResolver.hpp) in your project  
*or*  
Compile the [IncludeResolverMain.cpp](IncludeResolverMain.cpp) with the command `g++ IncludeResolverMain.cpp -o IncludeResolverMain.exe -std=c++17`.

c++17 or later compilation required.  
No external dependencies.

# Example

*Content of [exampleMain.cpp](exampleMain.cpp):*

```cpp
#include <iostream>

#include "IncludeResolver.hpp"

int main()
{
	IncludeResolverSettings settings;
	settings.includeFolderList.push_back(
		R"(C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include)");
	settings.toParseFolderList.push_back("example");
	settings.resolveFolderList.push_back(".");

	IncludeResolverResult result = include_resolver::computeIncludeResolve(settings, include_resolver::displayParseStatus);

	std::cout << "unresolvedIncludeSet: " << std::endl;
	for (const auto& unresolvedInclude : result.unresolvedIncludeSet) std::cout << unresolvedInclude << std::endl;

	std::cout << "resolveIncludeFolderSet: " << std::endl;
	for (const auto& resolveIncludeFolder : result.resolveIncludeFolderSet) std::cout << resolveIncludeFolder << std::endl;

	return 0;
}
```

The simple main above is pretty equivalent to the command line below:
```shell
> IncludeResolver.exe --toParse "example" --include "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include" --resolve "."
```

Output:
```
[1/2] example/incl/A.h
[2/2] example/src/A.cpp
[3/3] ./example/incl/A.h

unresolvedIncludeSet:
example/src/A.cpp:2 : B.h

resolveIncludeFolderSet:
./example/incl
C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.29.30133/include
```

[Bigger example of output](out.txt)

# Usage

## Library Usage

*Most important:*
```cpp
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

#define PARSE_STATUS_PARAM size_t current, size_t total, const PrettyPath& filePath
// parameters are captures
#define PARSE_STATUS_LAMBDA(...) [__VA_ARGS__](PARSE_STATUS_PARAM)
using ParseStatusCallback = std::function<void(PARSE_STATUS_PARAM)>;

namespace include_resolver
{
	// compute all folders to include in order to resolve all includes
	static IncludeResolverResult computeIncludeResolve(
		const IncludeResolverSettings& settings,
		ParseStatusCallback parseStatusCallback = [](size_t, size_t, const PrettyPath&) {});

	namespace example
	{
		// example of parse status callback that displays the parse status
		static void displayParseStatus(PARSE_STATUS_PARAM);
	} // namespace example
} // namespace include_resolver
```

<details>
  <summary><b>Show rest</b></summary>
 
```cpp
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

namespace std
{
	template <> struct hash<std::filesystem::path>
	{
		std::size_t operator()(const std::filesystem::path& path) const;
	};
} // namespace std
```
</details>

## Executable Usage

```
Usage :
IncludeResolver.exe [Settings...]
  Settings :
    --toParse [toParseFolder...] : list of folders to parse
    --include [includeFolder...] : list of folders used as include
    --resolve [resolveFolder...] : list of folders used to resolve includes
    --output [outputFile] : set the outputFile
  Other :
    --file [filePath...] : append the content of all files as arguments of the command line
    --verbose : enable the log
```

# Licence

MIT Licence. See [LICENSE file](LICENSE).
Please refer me with:

	Copyright (c) Nicolas VENTER All rights reserved.
