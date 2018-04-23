Instructions to test the program:

For generating the 1mfile: truncate -s 1G 1mfile
For generating the libmymem.o file: g++ -std=c++11 -c libmymem.cpp -lpthread
For generating the library: g++ -Wall -Werror -fpic -c libmymem.o -I . libmymem.cpp -std=c++11 -lpthread
For linking the libmymem.so file: g++ -shared -o libmymem.so libmymem.o
For exporting the path of the library path: export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.

For compiling the utility(memutil): g++ -I . -L . -Wall -o memutil memutil.cpp -l mymem -std=c++11 -lpthread
For running the utility: ./memutil -n 5

For compiling the utility(memutil1): g++ -I . -L . -Wall -o memutil memutil1.cpp -l mymem -std=c++11 -lpthread
For running the utility: ./memutil -n 100 -t 2
