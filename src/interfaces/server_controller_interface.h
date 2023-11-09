#ifndef MEMSESS_I_SERVER_CONTROLLER
#define MEMSESS_I_SERVER_CONTROLLER
 
namespace memsess::i {
    class ServerControllerInterface {
        public:
            virtual const char *parse( const char *data, unsigned int length ) = 0;
            virtual void interval() = 0;
    };
}

#endif
