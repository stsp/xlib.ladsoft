#include "cl.h"

typedef struct _ITEM {
    struct _ITEM *next; // ptr on next
    int  what;          // what is ITEM
    int  temp;          // temporary ?
    char data[1];       // data
} ITEM;

static ITEM *beg = NULL;
static ITEM *end = NULL;

void add( int what, int temp, char *data ) {
    ITEM *p = (ITEM *)malloc( sizeof( ITEM ) + strlen( data ) );

    if ( p != NULL ) {
        p->next = NULL;
        p->temp = temp;
        p->what = what;
        strcpy( p->data, data );
        if ( end == NULL )
            beg = end = p;
        else {
            end->next = p;
            end = p;
        }
    }
    else {
        fprintf( stderr, "Out of memory !\n" );
        exit(-1);
    }
}

void clear( void ) {
    ITEM *p;

    while( beg != NULL ) {
        p = beg;
        beg = beg->next;
        free( p );
    }
}

void iter( int what, void (*fiter)( char *, int ) ) {
    ITEM *p;

    for( p = beg; p != NULL; p = p->next )
        if ( p->what == what )
            (*fiter)( p->data, p->temp );
}

void iter0( int what, void (*fiter)( char *) ) {
    ITEM *p;

    for( p = beg; p != NULL; p = p->next )
        if ( p->what == what )
            (*fiter)( p->data );
}

int find( int what, char *data ) {
    ITEM *p;

    for( p = beg; p != NULL; p = p->next )
        if ( p->what == what && strcmp( p->data, data ) == 0 )
            return 1;
    return 0;
}

void print(void) {
    ITEM *p;

    printf( "\n%4s %4s %s\n", "TYPE", "TEMP", "DATA" );
    for( p = beg; p != NULL; p = p->next )
        printf( "%4d %4d %s\n", p->what, p->temp, p->data );
}
