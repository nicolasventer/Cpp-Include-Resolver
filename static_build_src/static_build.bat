del -f include_resolver.a
g++ *.cpp -c
ar rvs include_resolver.a *.o
del *.o
