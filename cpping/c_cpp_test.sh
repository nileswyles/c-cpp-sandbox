gcc -c ../c_lib/reader.c
g++ -c main-blah.cpp
g++ -o test.out reader.o main-blah.o

# if no extern "C", in the estream.h file when main-blah.cpp is compiled (in other words, when estream.h is included in cpp), then the link step (last command) will fail...
# TODO: automate this using make?