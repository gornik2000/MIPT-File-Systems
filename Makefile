all: ps lsof arg0 ext2

ps: my_ps.cpp error.hpp
	g++ my_ps.cpp -o ps

lsof: my_lsof.cpp error.hpp
	g++ my_lsof.cpp -o lsof

argv0: arg0.cpp error.hpp
	g++ arg0.cpp -o arg0

ext2: ext2.cpp error.hpp
	g++ ext2.cpp -o ext2