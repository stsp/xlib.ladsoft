#ifndef C_TYPES_H
#define C_TYPES_H

enum Basictype
{
    /* keep this ordering and dont insert anything before the end of the
     * basic types, type comparisons (LOSTCONV) depends on the ordering,
     * and the debug info has a table indexed by type
     */
    BT_BIT, BT_BOOL, BT_CHAR, BT_UNSIGNEDCHAR, BT_SHORT, BT_UNSIGNEDSHORT, BT_WCHAR_T, BT_ENUM,
        BT_INT, BT_UNSIGNEDINT, BT_LONG, BT_UNSIGNEDLONG, BT_LONGLONG,
        BT_UNSIGNEDLONGLONG, BT_FLOAT, BT_DOUBLE, BT_LONGDOUBLE, BT_FLOATIMAGINARY,
        BT_DOUBLEIMAGINARY, BT_LONGDOUBLEIMAGINARY,
		BT_FLOATCOMPLEX, BT_DOUBLECOMPLEX, BT_LONGDOUBLECOMPLEX, 
    /* end of basic types */
    BT_VOID, 
    /* end of debug needs */
    BT_UNTYPED, BT_TYPEDEF, BT_POINTER, BT_REF, BT_FARPOINTER, BT_STRUCT,
        BT_UNION, BT_FUNC, BT_CLASS, BT_ICLASS, BT_IFUNC, BT_MATCHALL,
        BT_MATCHNONE, BT_ELLIPSE, BT_BITFIELD, BT_MEMBERPTR, BT_DEFUNC, BT_COND,
        BT_CONSPLACEHOLDER, BT_TEMPLATEPLACEHOLDER, BT_SEGPOINTER, BT_STRING
};

typedef struct _type_t
{
	struct _type_t *link;
	struct _symbol_t *symbols;
	char *name;
	char *typedefname;
	enum Basictype btype;
	int elements;
	int t_const:1;
	int t_volatile:1;
	int t_restrict:1;
	int byValue:1;
} type_t;

#endif C_TYPES_H