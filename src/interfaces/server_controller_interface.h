#ifndef MEMSESS_I_SERVER_CONTROLLER
#define MEMSESS_I_SERVER_CONTROLLER
 
namespace memsess::i {
    class ServerControllerInterface {
        public:
            struct Result {
                char *data;
                unsigned int length;
            };
            virtual void parse( const char *data, unsigned int length, Result &result ) = 0;
            virtual void interval() = 0;
    };
}

#endif
