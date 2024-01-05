#include "cl.h"

void main( int argc, char *argv[] ) {
    if ( argc == 1 )
        help();

    config( argv[0], "MK386", argc - 1, argv + 1 );

    atexit( clear );

    if ( nofiles )
        help();
//    print();
    make();
}
