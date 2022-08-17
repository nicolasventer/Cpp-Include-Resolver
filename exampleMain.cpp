#include <iostream>

#include "IncludeResolver.hpp"

int main()
{
	IncludeResolverSettings settings;
	settings.includeFolderList.push_back(
		R"(C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include)");
	settings.toParseFolderList.push_back("example");
	settings.resolveFolderList.push_back(".");

	IncludeResolverResult result
		= include_resolver::computeIncludeResolve(settings, include_resolver::example::displayParseStatus);

	std::cout << '\n';
	std::cout << "unresolvedIncludeSet: " << std::endl;
	for (const auto& unresolvedInclude : result.unresolvedIncludeSet) std::cout << unresolvedInclude << std::endl;

	std::cout << '\n';
	std::cout << "resolveIncludeFolderSet: " << std::endl;
	for (const auto& resolveIncludeFolder : result.resolveIncludeFolderSet) std::cout << resolveIncludeFolder << std::endl;

	return 0;
}