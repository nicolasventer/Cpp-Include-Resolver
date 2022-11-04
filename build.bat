DEL *.o
DEL *.dll
DEL *.exe

g++ -c IncludeResolver.cpp || goto :EOF
g++ -c libs/json11/json11.cpp || goto :EOF
g++ -c main.cpp || goto :EOF
g++ -o IncludeResolver.exe main.o IncludeResolver.o json11.o || goto :EOF

@REM g++ -shared -fPIC -o IncludeResolver.dll IncludeResolver.o json11.o || goto :EOF
@REM g++ -shared -fPIC -static -o IncludeResolver_static.dll IncludeResolver.o json11.o || goto :EOF
