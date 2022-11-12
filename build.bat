@REM DEL *.dll
@REM DEL *.exe

g++ -c IncludeResolver.cpp || goto :EOF
g++ -c libs/json11/json11.cpp || goto :EOF
g++ -o IncludeResolver.exe IncludeResolver.o json11.o || goto :EOF

@REM g++ -shared -fPIC -static -o IncludeResolver.dll IncludeResolver.cpp json11.o -DNO_MAIN || goto :EOF

DEL *.o
