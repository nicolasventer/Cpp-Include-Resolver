# Include Resolver

# Description

IncludeResolver is an **executable** that allows you to **compute all folders to add to `includePath`** in order to resolve all includes.

IncludeResolver can also be used as an **dll**.

**Explanation `includePath` by example:**  
[example/src/A.cpp](example/src/A.cpp) contains	 `#include "A.h"`.  
In order to find the file [example/incl/A.h](example/incl/A.h), g++ need to but executed with the argument `-Iexample/incl`.  
So the Include Resolver will prompt the folder `example/incl`.

# Features

- retrieve the folders to include
- detect the conflicts
- store unresolved includes
- write the output result in a file (json format)

# Dependencies

- [json11](https://github.com/dropbox/json11) (included in the project)

# Installation

Build executable:

```bash
g++ -o IncludeResolver.exe main.cpp IncludeResolver.cpp libs/json11/json11.cpp
```

Build dll:

```bash
g++ -shared -fPIC -o IncludeResolver.dll main.cpp IncludeResolver.cpp libs/json11/json11.cpp
```

c++17 or later compilation required. *(it can be specified with the flag `-std=c++17`)*

# Example

```bash
IncludeResolver.exe --toParse example --resolve example --include "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.33.31629\include"
```

Output:
```
{"conflictedIncludes": [{"canBeResolvedBy": ["D:/Projets/C++/Cpp-Include-Resolver-main/example/incl", "D:/Projets/C++/Cpp-Include-Resolver-main/example/incl/C"], "include": "C.h", "includedBy": [{"filePath": "D:/Projets/C++/Cpp-Include-Resolver-main/example/src/A.cpp", "line": "3"}]}], "invalidPaths": [], "resolveIncludeFolders": ["C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.33.31629/include", "D:/Projets/C++/Cpp-Include-Resolver-main/example/incl"], "unresolvedIncludes": [{"filePath": "D:/Projets/C++/Cpp-Include-Resolver-main/example/src/A.cpp", "include": "B.h", "line": "2"}]}
```

<!-- TODO: remove this example ? -->
[Bigger example of output with UE5](out.json)  
*Result of the parse and resolve of `UnrealEngine-5.0.3-release/Engine/Source/Editor`  
Executed command: `IncludeResolver.exe --toParse . --resolve . --include "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.33.31629\include" --output out.json --verbose`*

# Usage

## Executable Usage

Usage:

```
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
    --help-result/-hr : display the json format of the result
```

ResolverResult json:

```js
{
	"invalidPaths": string[],
	"unresolvedIncludes": { "filePath": string, "line": number, "include": string }[],
	"conflictedIncludes": { "include": string, "includedBy": { "filePath": string, "line": number }[], "canBeResolvedBy": string[] }[],
	"resolveIncludeFolders": string[]
}
```
## Library Usage

```cpp
// same as the executable
extern "C" __declspec(dllexport) int includeResolverMain(int argc, const char* argv[]);
```

# Licence

MIT Licence. See [LICENSE file](LICENSE).
Please refer me with:

	Copyright (c) Nicolas VENTER All rights reserved.
