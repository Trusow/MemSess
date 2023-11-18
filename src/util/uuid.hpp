#ifndef MEMSESS_UTIL_UUID 
#define MEMSESS_UTIL_UUID 

#include <random>

namespace memsess::util {
    class UUID {
        private:
            static char convertIntToHex( unsigned int value );
            static unsigned int getInt( char val );
            static char getChar( unsigned int val );
        public:
            static const unsigned int LENGTH = 36;
            static const unsigned int LENGTH_RAW = 16;
            static void generate( char *data );
            static bool toBin( const char *data, char *resultData );
            static bool toNormal( const char *data, char *resultData );
    };

    char UUID::convertIntToHex( unsigned int value ) {
        if( value >= 0 && value <= 9 ) {
            return value + 48;
        } else if( value >= 10 && value <= 15 ) {
            return value + 87;
        }

        return 48;
    }

    void UUID::generate( char *data ) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        for (int i = 0; i < LENGTH; i++) {
            if( i == 8 || i == 13 || i == 18 || i == 23 ) {
                data[i] = '-';
            } else if( i == 14 ) {
                data[i] = '4';
            } else if( i == 19 ) {
                data[i] = convertIntToHex( dis2( gen ) );
            } else {
                data[i] = convertIntToHex( dis( gen ) );
            }
        }
    }

    unsigned int UUID::getInt( char val ) {
        switch( val ) {
            case '0':
                return 0;
            case '1':
                return 1;
            case '2':
                return 2;
            case '3':
                return 3;
            case '4':
                return 4;
            case '5':
                return 5;
            case '6':
                return 6;
            case '7':
                return 7;
            case '8':
                return 8;
            case '9':
                return 9;
            case 'a': case 'A':
                return 10;
            case 'b': case 'B':
                return 11;
            case 'c': case 'C':
                return 12;
            case 'd': case 'D':
                return 13;
            case 'e': case 'E':
                return 14;
            case 'f': case 'F':
                return 15;
            default:
                return -1;
        }
    }

    char UUID::getChar( unsigned int val ) {
        switch( val ) {
            case 0:
                return '0';
            case 1:
                return '1';
            case 2:
                return '2';
            case 3:
                return '3';
            case 4:
                return '4';
            case 5:
                return '5';
            case 6:
                return '6';
            case 7:
                return '7';
            case 8:
                return '8';
            case 9:
                return '9';
            case 10:
                return 'a';
            case 11:
                return 'b';
            case 12:
                return 'c';
            case 13:
                return 'd';
            case 14:
                return 'e';
            case 15:
                return 'f';
            default:
                return 0;
        }
    }

    bool UUID::toBin( const char *data, char *resultData ) {
        bool isEven = true;
        unsigned int iAdd = 0;
        int val = 0;
        unsigned char parseVal = 0;

        for( unsigned int i = 0; i < LENGTH; i++ ) {

            if( i == 8 || i == 13 || i == 18 || i == 23 ) {
                continue;
            }

            parseVal = getInt( data[i] );

            if( parseVal < 0 ) {
                return false;
            }

            if( isEven ) {
                val = parseVal * 16;
            } else {
                val += parseVal;

                resultData[iAdd] = val;
                val = 0;
                iAdd++;
            }

            isEven = !isEven;
        }

        return true;
    }

    bool UUID::toNormal( const char *data, char *resultData ) {
        unsigned int offset = 0;
        unsigned int offsetSeparator = 0;

        for( unsigned int i = 0; i < LENGTH_RAW; i++ ) {
            unsigned char ch = data[i];


            if( ch < 16 ) {
                resultData[offset+offsetSeparator] = '0';
                resultData[offset+offsetSeparator+1] = getChar( ch );
            } else {
                resultData[offset+offsetSeparator] = getChar( ch / 16 );
                resultData[offset+offsetSeparator+1] = getChar( ch % 16 );
            }

            if( i == 3 || i == 5 || i == 7 || i == 9 ) {
                resultData[offset+offsetSeparator+2] = '-';
                offsetSeparator++;
            }

            offset += 2;
        }

        return resultData[14] == '4';
    }
}

#endif
