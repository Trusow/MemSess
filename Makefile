FLAGS = -ldl -pthread -L/usr/lib/ -std=c++2a -s -O3 -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -levent -static

multi:
	g++ memsess-server.cpp \
	\
	$(FLAGS) -D MEMSESS_MULTI -o ./bin/memsess-multi

mono:
	g++ bard-server.cpp \
	\
	$(FLAGS) -o ./bin/memsess-mono
