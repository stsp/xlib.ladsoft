#include "c_types.h"

#define is_assign_operator(tok) \
	(TokMap[tok] & TOK_ASSGNOP)
#define is_type_name(tok) \
	((TokMap[tok] & TOK_DECL_SPEC) || tok == IDENTIFIER && Cursym != 0 && Cursym->object_type == OBJ_TYPEDEF_NAME)
#define is_declaration(tok) \
	is_type_name(tok)
#ifdef C89
#define is_external_declaration(tok) \
	(is_declaration(tok) || tok == STAR || tok == LPAREN || tok == IDENTIFIER)
#else
#define is_external_declaration(tok) \
	is_declaration(tok)
#endif
#define is_expression(tok) \
	(TokMap[tok] & TOK_EXPR)
#define is_statement(tok) \
	(TokMap[tok] & TOK_STMT)
#define is_function_body(tok) \
	(tok == LBRACE || (is_declaration(tok) && tok != TYPEDEF))


#if 1
#define TRACEIN(s) do { if (DebugLevel == 2) printf("%*s%s {\n", TraceLevel, "", s); fflush(stdout); TraceLevel++; } while(0);
#define TRACEOUT(s) do { --TraceLevel; if (DebugLevel == 2) printf("%*s} (%s)\n", TraceLevel, "", s); fflush(stdout); } while(0);
#else
#define TRACEIN(s) 
#define TRACEOUT(s)
#endif

enum {
	TOK_UNKNOWN = 0,
	TOK_EXPR = 1,
	TOK_CONSTANT = 2,
	TOK_TYPE_SPECIFIER_QUALIFIER = 4,
	TOK_TYPE_SPECIFIER = 8,
	TOK_TYPE_QUALIFIER = 16,
	TOK_TAG = 32,
	TOK_STRUCT = 64,
	TOK_STORAGE_CLASS = 256,
	TOK_STMT = 1024,
	TOK_DECL_SPEC = 2048,
	TOK_ASSGNOP = 4096,
};

enum {
	LEVEL_GLOBAL = 0,
	LEVEL_FUNCTION = 1,
	LEVEL_STATEMENT = 2
};

enum {
	OBJ_TYPEDEF_NAME = 1,
	OBJ_FUNCTION_DECL = 2,
	OBJ_FUNCTION_DEFN = 4,
	OBJ_ENUMERATOR = 8,
	OBJ_VARIABLE = 16,
	OBJ_PARAMETER = 32,
	OBJ_TYPE = 64,
	OBJ_LABEL = 128,
	OBJ_IDENTIFIER = (2+4+8+16+32+64+128)
};

typedef enum {
	SC_NONE, SC_STATIC, SC_EXTERNAL, SC_AUTO, SC_GLOBAL, SC_TYPE,
	SC_CONSTANT, SC_LABEL, SC_MEMBER
} Storageclass;

typedef struct symbol_t {
	link_t link;
	const char *name;
	type_t *type;
	Storageclass storage_class;
	int object_type;
	void *data;
	int startline;
	int endline;
	const char *file;
} symbol_t;

#define HASH_SIZE 1021

typedef struct _symtab_t{
	link_t link;
	int level;		/* nesting level */
	list_t symbols;		/* list of symbols */
	list_t *hashed_symbols;
} symtab_t;

symtab_t *
new_symbol_table(list_t *owner, bool hashed);
symbol_t *
find_symbol(symtab_t *tab, const char *name, int all_scope);
symbol_t *
install_symbol(symtab_t *tab, const char *name, 
			   Storageclass storage_class, int object_type);
const char *
object_name(int object_type);


extern symtab_t *Cursymtab;		/* current symbol table */
extern int DebugLevel ;
extern bool prm_c99;
extern bool prm_loose;