# Include Resolver

# Description

IncludeResolver is an **executable** that allows you to **compute all folders to add to `includePath`** in order to resolve all includes.

IncludeResolver can also be used as an **dll**, (see [build.bat](build.bat) and [test_include_resolver.py](test_include_resolver.py) for detailed example).

**Explanation `includePath` by example:**  
[example/src/A.cpp](example/src/A.cpp) contains	 `#include "A.h"`.  
In order to find the file [example/incl/A.h](example/incl/A.h), g++ need to but executed with the argument `-Iexample/incl`.  
So the Include Resolver will prompt the folder `example/incl`.

# Features

- retrieve the folders to include
- detect the conflicts
- store unresolved includes
- write the output result in a file (yaml format)

# Installation

No external dependencies.

Build executable:

```bash
g++ -o IncludeResolver.exe main.cpp IncludeResolver.cpp
```

c++17 or later compilation required. *(it can be specified with the flag `-std=c++17`)*

# Example

```bash
IncludeResolver.exe --toParse example --resolve example --include "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.33.31629\include"
```

Output:
```yaml
invalidPaths:
unresolvedIncludes:
    D:/Projets/Cpp-Include-Resolver/example/src/A.cpp:2 : B.h
conflictedIncludes:
    C.h:
        includedBy:
            - D:/Projets/Cpp-Include-Resolver/example/src/A.cpp:3
        canBeResolvedBy:
            - D:/Projets/Cpp-Include-Resolver/example/incl
            - D:/Projets/Cpp-Include-Resolver/example/incl/C
resolveIncludeFolders:
    - C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.33.31629/include
    - D:/Projets/Cpp-Include-Resolver/example/incl
```

[Bigger example of output with UE5](out.yaml)  
*Result of the parse and resolve of `C:\Program Files\Epic Games\UE_5.1\Engine\Source\Editor`  
Executed command: `IncludeResolver.exe --toParse . --resolve . --include "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.33.31629\include" --output out.yaml --verbose`*

# Usage

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
    --help-result/-hr : display the yaml format of the result
```

ResolverResult yaml:

```js
{
	"invalidPaths": string[],
	"unresolvedIncludes": Object{ ["filePath": string]: string },
	"conflictedIncludes": Object{ ["include": string]: { "includedBy": string[], "canBeResolvedBy": string[] } },
	"resolveIncludeFolders": string[]
}
```

# Licence

MIT Licence. See [LICENSE file](LICENSE).
Please refer me with:

	Copyright (c) Nicolas VENTER All rights reserved.
