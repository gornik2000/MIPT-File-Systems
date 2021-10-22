all: ps lsof arg0

ps: my_ps.cpp
	g++ my_ps.cpp -o ps

lsof: my_lsof.cpp
	g++ my_lsof.cpp -o lsof

argv0: arg0.cpp
	g++ arg0.cpp -o arg0