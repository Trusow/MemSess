#ifndef MEMSESS_UTIL_SERIALIZATION
#define MEMSESS_UTIL_SERIALIZATION

#include <string.h>
#include <arpa/inet.h>
#include <memory>

namespace memsess::util {
    class Serialization {
        public:
            enum Type {
                CHAR,
                STRING,
                FIXED_STRING,
                STRING_WITH_NULL,
                SHORT_INT,
                INT,
                END,
            };

            struct Item {
                Type type;
                char value_char;
                const char *value_string;
                short int value_short_int;
                int value_int;
                unsigned int length;
            };


            static std::unique_ptr<char[]> pack( const Item **items, unsigned int &resultLength );
            static bool unpack( Item **items, const char *data, unsigned int length );
    };

    std::unique_ptr<char[]> Serialization::pack( const Item **items, unsigned int &resultLength ) {
        resultLength = 0;

        for( unsigned int i = 0; items[i]->type != END; i++ ) {
            auto item = items[i];

            switch( item->type ) {
                case CHAR:
                    resultLength += sizeof( char );
                    break;
                case STRING:
                    resultLength += item->length + sizeof( int );
                    break;
                case FIXED_STRING:
                    resultLength += item->length;
                    break;
                case STRING_WITH_NULL:
                    resultLength += strlen( item->value_string ) + 1;
                    break;
                case SHORT_INT:
                    resultLength += sizeof( short int );
                    break;
                case INT:
                    resultLength += sizeof( int );
                    break;
            }
        }

        unsigned int offset = 0;
        unsigned int length = 0;
        int value_int = 0;
        short int value_short_int = 0;
        char null = 0;
        auto data = std::make_unique<char[]>(resultLength);

        for( unsigned int i = 0; items[i]->type != END; i++ ) {
            auto item = items[i];

            switch( item->type ) {
                case CHAR:
                    data[offset] = item->value_char;
                    offset += sizeof( char );
                    break;
                case STRING:
                    length = htonl( item->length );

                    memcpy( &data[offset], &length, sizeof( int ) );
                    offset += sizeof( int );

                    memcpy( &data[offset], item->value_string, item->length );
                    offset += item->length;
                    break;
                case FIXED_STRING:
                    memcpy( &data[offset], item->value_string, item->length );
                    offset += item->length;
                    break;
                case STRING_WITH_NULL:
                    length = strlen( item->value_string );

                    memcpy( &data[offset], item->value_string, length );
                    offset += length;

                    memcpy( &data[offset], &null, 1 );
                    offset += 1;
                    break;
                case SHORT_INT:
                    value_short_int = htons( item->value_short_int );

                    memcpy( &data[offset], &value_short_int, sizeof( short int ) );
                    offset += sizeof( short int );
                    break;
                case INT:
                    value_int = htonl( item->value_int );

                    memcpy( &data[offset], &value_int, sizeof( int ) );
                    offset += sizeof( int );
                    break;
            }
        }

        return data;
    }

    bool Serialization::unpack( Item **items, const char *data, unsigned int length ) {
        int value_int = 0;
        short int value_short_int = 0;
        unsigned int string_length;
        unsigned int offset = 0;

        for( unsigned int i = 0; items[i]->type != END; i++ ) {
            if( offset == length ) {
                return false;
            }

            auto item = items[i];

            switch( item->type ) {
                case CHAR:
                    item->value_char = data[offset];
                    offset += sizeof( char );
                    break;
                case STRING:
                    if( offset + sizeof( int ) > length ) {
                        return false;
                    }

                    memcpy( &string_length, &data[offset], sizeof( int ) );
                    string_length = ntohl( string_length );
                    offset += sizeof( int );

                    if( offset + string_length > length ) {
                        return false;
                    }

                    item->value_string = &data[offset];
                    item->length = string_length;
                    offset += string_length;
                    break;
                case FIXED_STRING:
                    string_length = item->length;
                    if( offset + string_length > length ) {
                        return false;
                    }

                    item->value_string = &data[offset];
                    offset += string_length;
                    break;
                case STRING_WITH_NULL:
                    item->value_string = &data[offset];

                    string_length = 0;
                    for( ; data[offset] != 0; offset++ ) {
                        string_length++;
                        if( offset == length  ) {
                            return false;
                        }
                    }

                    if( offset + 1 > length ) {
                        return false;
                    }

                    offset += 1;
                    item->length = string_length;

                    break;
                case SHORT_INT:
                    if( offset + sizeof( short int ) > length ) {
                        return false;
                    }

                    memcpy( &value_short_int, &data[offset], sizeof( short int ) );
                    item->value_short_int = ntohs( value_short_int );
                    offset += sizeof( short int );
                    break;
                case INT:
                    if( offset + sizeof( int ) > length ) {
                        return false;
                    }

                    memcpy( &value_int, &data[offset], sizeof( int ) );
                    item->value_int = ntohl( value_int );
                    offset += sizeof( int );
                    break;
            }
        }

        return offset == length;
    }
}
 
#endif
