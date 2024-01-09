FLAGS = -ldl -pthread -L/usr/lib/ -std=c++2a -s -O3 -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -levent -static

MKDIR = mkdir bin -p

multi:
	$(MKDIR) && \
	g++ memsess-server.cpp \
	\
	$(FLAGS) -D MEMSESS_MULTI -o ./bin/memsess-multi

mono:
	$(MKDIR) && \
	g++ memsess-server.cpp \
	\
	$(FLAGS) -o ./bin/memsess-mono
