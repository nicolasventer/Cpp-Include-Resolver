@REM DEL *.dll
@REM DEL *.exe

g++ -c IncludeResolver.cpp || goto :EOF
g++ -o IncludeResolver.exe IncludeResolver.o || goto :EOF

@REM g++ -shared -fPIC -static -o IncludeResolver.dll IncludeResolver.cpp -DNO_MAIN || goto :EOF

DEL *.o
