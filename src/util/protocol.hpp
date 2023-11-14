#ifndef MEMSESS_UTIL_PROTOCOL
#define MEMSESS_UTIL_PROTOCOL

#include <string.h>
#include <arpa/inet.h>

namespace memsess::util {
    class Protocol {
        public:
            enum TypeEncode {
                ENC_STRING,
                ENC_STRICT_STRING,
                ENC_STRING_WITH_NULL,
                ENC_SHORT_INT,
                ENC_INT,
                ENC_END,
            };

            enum TypeDecode {
                DEC_STRING,
                DEC_STRING_WITH_NULL,
                DEC_SHORT_INT,
                DEC_INT,
                DEC_END,
            };

            struct ItemEncode {
                TypeEncode type;
                const char *value_string;
                short int value_short_int;
                int value_int;
                unsigned int length;
            };

            struct ItemDecode {
                TypeDecode type;
                unsigned int min;
                unsigned int max;
                const char *value_string;
                short int value_short_int;
                int value_int;
                unsigned int length;
            };

            char *encode( const ItemEncode *items, unsigned int &resultLength );
            bool decode( ItemDecode **items, const char *data, unsigned int length );
    };

    char *Protocol::encode( const ItemEncode *items, unsigned int &resultLength ) {
        resultLength = 0;

        for( unsigned int i = 0; items[i].type != ENC_END; i++ ) {
            auto item = items[i];

            switch( item.type ) {
                case ENC_STRING:
                    resultLength += item.length + sizeof( int );
                    break;
                case ENC_STRICT_STRING:
                    resultLength += item.length;
                    break;
                case ENC_STRING_WITH_NULL:
                    resultLength += strlen( item.value_string ) + 1;
                    break;
                case ENC_SHORT_INT:
                    resultLength += sizeof( short int );
                    break;
                case ENC_INT:
                    resultLength += sizeof( int );
                    break;
            }
        }

        unsigned int offset = 0;
        unsigned int length = 0;
        int value_int = 0;
        short int value_short_int = 0;
        char null = 0;
        char *data = new char[resultLength];

        for( unsigned int i = 0; items[i].type != ENC_END; i++ ) {
            auto item = items[i];

            switch( item.type ) {
                case ENC_STRING:
                    length = htonl( item.length );

                    memcpy( &data[offset], &length, sizeof( int ) );
                    offset += sizeof( int );

                    memcpy( &data[offset], item.value_string, item.length );
                    offset += item.length;
                    break;
                case ENC_STRICT_STRING:
                    memcpy( &data[offset], item.value_string, item.length );
                    offset += item.length;
                    break;
                case ENC_STRING_WITH_NULL:
                    length = strlen( item.value_string );

                    memcpy( &data[offset], item.value_string, length );
                    offset += length;

                    memcpy( &data[offset], &null, 1 );
                    offset += 1;
                    break;
                case ENC_SHORT_INT:
                    value_short_int = htons( item.value_short_int );

                    memcpy( &data[offset], &value_short_int, sizeof( short int ) );
                    offset += sizeof( short int );
                    break;
                case ENC_INT:
                    value_int = htonl( item.value_int );

                    memcpy( &data[offset], &value_int, sizeof( int ) );
                    offset += sizeof( int );
                    break;
            }
        }

        return data;
    }

    bool Protocol::decode( ItemDecode **items, const char *data, unsigned int length ) {
        int value_int = 0;
        short int value_short_int = 0;
        unsigned int string_length;
        unsigned int offset = 0;

        for( unsigned int i = 0; items[i]->type != DEC_END; i++ ) {
            if( offset == length ) {
                return false;
            }

            auto item = items[i];

            switch( item->type ) {
                case DEC_STRING:
                    if( item->min == item->max ) {
                        string_length = item->min;
                    } else {
                        if( offset + sizeof( int ) > length ) {
                            return false;
                        }
                        memcpy( &string_length, &data[offset], sizeof( int ) );
                        string_length = ntohl( string_length );
                        offset += sizeof( int );
                    }

                    if( offset + string_length > length ) {
                        return false;
                    }

                    item->value_string = &data[offset];
                    item->length = string_length;
                    offset += string_length;
                    break;
                case DEC_STRING_WITH_NULL:
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
                case DEC_SHORT_INT:
                    if( offset + sizeof( short int ) > length ) {
                        return false;
                    }

                    memcpy( &value_short_int, &data[offset], sizeof( short int ) );
                    item->value_short_int = ntohs( value_short_int );
                    offset += sizeof( short int );
                    break;
                case DEC_INT:
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
