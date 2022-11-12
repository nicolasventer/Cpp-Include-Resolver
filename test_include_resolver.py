# Load dll IncludeResolver.dll
import ctypes
import os
import sys

# Get the path of the dll file
dll_path = os.path.abspath('IncludeResolver.dll')
# Load the dll file
dll = ctypes.cdll.LoadLibrary(dll_path)

# int include_resolver_main(int argc, const char* argv[]);

# build the argv from sys.argv
argv = (ctypes.c_char_p * len(sys.argv))()
for i, arg in enumerate(sys.argv):
    argv[i] = ctypes.c_char_p(arg.encode('utf-8'))

# Call the function

ret = dll.include_resolver_main(len(sys.argv), argv)
exit(ret)
