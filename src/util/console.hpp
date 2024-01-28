#ifndef MEMSESS_UTIL_CONSOLE
#define MEMSESS_UTIL_CONSOLE

#include <iostream>

namespace memsess::util {
    class Console {
        public:
            static void printSuccess( const char *text );
            static void printDanger( const char *text );
    };

    void Console::printSuccess( const char *text ) {
        std::cout << "\x1b[32m" << text << "\x1b[0m" << std::endl;;
    }

    void Console::printDanger( const char *text ) {
        std::cout << "\x1b[31m" << text << "\x1b[0m" << std::endl;;
    }
}

#endif
