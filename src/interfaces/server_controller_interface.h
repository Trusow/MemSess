#ifndef MEMSESS_I_SERVER_CONTROLLER
#define MEMSESS_I_SERVER_CONTROLLER

#include <memory>
 
namespace memsess::i {
    class ServerControllerInterface {
        public:
            virtual std::unique_ptr<char[]> parse(
                const char *data,
                unsigned int length,
                unsigned int &resultLength
            ) = 0;
            virtual void interval() = 0;
    };
}

#endif
