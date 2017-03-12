UNAME_S := $(shell uname -s)

FLAGS = -std=c++14 -g -O0 -fPIC -D_REENTRANT=1 -fno-omit-frame-pointer # -fsanitize=address -fno-common 

ifeq ($(UNAME_S),Linux)
all:
	clang++ $(FLAGS) -c gnuwrapper.cpp
	clang++ $(FLAGS) -c driver.cpp
	clang++ $(FLAGS) -shared gnuwrapper.o driver.o -Bsymbolic -o libgcmalloc.so -ldl -lpthread
	clang++ -std=c++14 -g testme.cpp -L. -lgcmalloc -o testme
endif

ifeq ($(UNAME_S),Darwin)
all:
	clang++ $(FLAGS) -c macwrapper.cpp
	clang++ $(FLAGS) -c driver.cpp
	# clang++ $(FLAGS) driver.cpp testme.cpp -o testme
	clang++ $(FLAGS) -compatibility_version 1 -current_version 1 -dynamiclib -install_name './libgcmalloc.dylib' macwrapper.o driver.o -o libgcmalloc.dylib
	clang++ -std=c++14 -g testme.cpp -L. -lgcmalloc -o testme
endif
