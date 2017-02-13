all:
	clang++ -std=c++14 -g -fPIC -D_REENTRANT=1 -c macwrapper.cpp
	clang++ -std=c++14 -g -fPIC -D_REENTRANT=1 -c driver.cpp
	clang++ -std=c++14 -g -fPIC -D_REENTRANT=1 -compatibility_version 1 -current_version 1 -dynamiclib -install_name './libstatsalloc.dylib' macwrapper.o driver.o -o libstatsalloc.dylib
	clang++ -std=c++14 -g testme.cpp -L. -lstatsalloc -o testme
