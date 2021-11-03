all: ext2 ps lsof arg0
#==========================================================

ps: my_ps.cpp error.o
	g++ -o ps.out my_ps.cpp error.o

lsof: my_lsof.cpp error.o
	g++ -o lsof.out my_lsof.cpp error.o

argv0: arg0.cpp error.o
	g++ -o arg0.out arg0.cpp error.o

ext2: main_ext2.cpp ext2.o malloc.o error.o
	g++ -o ext2.out main_ext2.cpp ext2.o error.o malloc.o

#===========================================================

error.o: error.hpp error.cpp
	g++ -c error.cpp error.hpp

malloc.o: malloc.hpp malloc.cpp
	g++ -c malloc.cpp malloc.hpp

ext2.o: ext2.hpp ext2.cpp
	g++ -c ext2.cpp ext2.hpp

#===========================================================
clean:
	rm -rf *.o *.out