// Copyright (c) Nicolas VENTER All rights reserved.

#pragma once

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
	--help-result/-hr : display the json format of the result

ResolverResult json:
{
	"invalidPaths": string[],
	"unresolvedIncludes": { "filePath": string, "line": number, "include": string }[],
	"conflictedIncludes": { "include": string, "includedBy": { "filePath": string, "line": number }[], "canBeResolvedBy": string[]
}[], "resolveIncludeFolders": string[]
}
*/

// same as the executable
extern "C" __declspec(dllexport) int includeResolverMain(int argc, const char* argv[]);
