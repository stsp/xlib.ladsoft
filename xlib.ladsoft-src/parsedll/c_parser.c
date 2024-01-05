/* c_parser.c - a C 99 parser */
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*  Copyright 2000, 2001 Dibyendu Majumdar.
 *  Author : Dibyendu Majumdar
 *  Email  : dibyendu@mazumdar.demon.co.uk  
 *  Website: www.mazumdar.demon.co.uk
 *
 */

/*
 * 12 Jan 2000 - Started
 * 13 Jan 2000 - Completed 1st version
 * 14-15 Jan 2000 - Fixed many bugs, added TRACE()
 * 16 Jan 2001 - Streamlined TokMap, and token test macros
 * 16 Jan 2001 - Documented FIRST sets, ambiguities
 * 17 Jan 2001 - Started using CVS
 * 17 Jan 2001 - Imported typedef handling from UPS sources
 * 18 Jan 2001 - First version that can parse typedefs
 *               BUG: a typedef name can be hidden by declaring a function,
 *                    variable, or enumerator with the same name. Currently
 *                    cannot handle this.
 * 19 Jan 2001 - Started work on C99 grammer.
 * 19 Jan 2001 - Added support for designators in initializers
 * 19 Jan 2001 - Modified external declaration so that these can no longer
 *               begin without a declaration specifier.
 * 19 Jan 2001 - Selection (if, switch) and iteration (for, do, while)
 *               statements now enclosed in blocks, sub statements also 
 *               enclosed in blocks.
 * 19 Jan 2001 - First expression of for can now be a declaration.
 * 19 Jan 2001 - Compound statements can now have declartions and statements
 *               interspersed.
 * 20 Jan 2001 - Tentative code for parsing compound literals.
 * 20 Jan 2001 - Structured trace messages to show parser in action.
 * 16 Nov 2001 - Tidied up for release. As you can see I have not worked on this since Jan, 2001.
 *               It is unlikely I will be able to spend time on this in the immediate future. Hence,
 *               I am releasing it "as is".
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "c_lex.h"
#include "c_list.h"
#include "c_parser.h"
#include "c_preprc.h"

void dump(void);

/***
* Various FIRST SETS
* ------------------
*   declarator
*        IDENTIFIER * ( 
*   declaration_specifier
*        AUTO REGISTER STATIC EXTERN TYPEDEF VOID CHAR
*        SHORT INT LONG FLOAT IDOUBLE SIGNED UNSIGNED STRUCT UNION
*        ENUM TYPEDEF_NAME CONST VOLATILE
*   declaration
*        declaration_specifier
*   external_declaration
*        declaration declarator
*   expression
*        ( ++ -- & * + - ~ ! SIZEOF IDENTIFIER FLOATING_CONSTANT
*        INTEGER_CONSTANT CHARACTER_CONSTANT STRING_CONSTANT
*   statement
*        expression ; { IF SWITCH WHILE DO FOR GOTO CONTINUE BREAK
*        RETURN CASE DEFAULT IDENTIFIER
*/

/***
* Ambiguity
*    1) A declaration and a function definition look alike.
*    2) An expression and a statement look alike.
*    3) A type cast and an expression look alike.
*    4) A labeled statment and an expression statement look alike.
*    5) An assignment and a conditional expression look alike.
*/


static unsigned long TokMap[BADTOK];
static unsigned long uid = 1;

int DebugLevel = 0;
static int TraceLevel = 0;
bool prm_loose = TRUE;
bool prm_c99 = TRUE;
static	int Level = 0;
static	int Saw_ident = 0;
static	int Is_func = 0;
static	token_t	tok;
static	int Parsing_struct = 0;
static	int Parsing_oldstyle_parmdecl = 0;
static	Storageclass Storage_class[100];
static	int stack_ptr = -1;
static  symbol_t *lastInstalledSymbol;
list_t identifiers;		/* head of the identifiers list */
list_t labels;
list_t types;

list_t exited_scopes;
list_t exited_types;

symtab_t *Cursymtab;		/* current symbol table */
static symtab_t *Curlabels;		
static symtab_t *Curtypes;		
static symtab_t *Curstruct;
static symbol_t *Cursym = 0;

static void
init_tokmap(void);

static void
init_symbol_table(void);

static void
enter_scope(void);

static void
exit_scope(void);


static void
match(token_t expected_tok);

static bool
check_not_typedef(void);

static void
constant_expression(void);

static void
expression(void);

static void
primary_expression(void);

static void
postfix_operator(void);

static void
postfix_operators(void);

static void
sizeof_expression(void);

static void
unary_expression(void);

static void
multiplicative_expression(void);

static void
additive_expression(void);

static void
shift_expression(void);

static void
relational_expression(void);

static void
equality_expression(void);

static void
and_expression(void);

static void
exclusive_or_expression(void);

static void
inclusive_or_expression(void);

static void
logical_and_expression(void);

static void
logical_or_expression(void);

static void
conditional_expression(void);

static void
assignment_expression(void);

static void
labeled_statement(void);

static void
case_statement(void);

static void
default_statement(void);

static void
if_statement(void);

static void
switch_statement(void);

static void
while_statement(void);

static void
do_while_statement(void);

static void
for_statement(void);

static void
break_statement(void);

static void
continue_statement(void);

static void
goto_statement(void);

static void
return_statement(void);

static void
empty_statement(void);

static void
expression_statement(void);

static void
statement(void);

static void
compound_statement(void);

static void
enumerator(type_t *type);

static type_t *
enum_specifier(void);

static void
member(type_t *type);

static void
members(void);

static type_t *
struct_or_union_specifier(bool isunion);

static void
type_name(void);

static type_t *
declaration_specifiers(int no_storage_class);

static type_t *
pointer(type_t *type, bool byValue);

static type_t *
direct_declarator(type_t *type, int abstract);

static void 
parameter_list(symtab_t *prototype, int *new_style);

static void
suffix_declarator(void);

static type_t *
declarator(type_t *type, int abstract);

static void
designator(void);

static void
initializer(int recurse);

static void
function_definition(void);

static int
init_declarator(type_t *type, int check_if_function);

static void
declaration(void);

static void
translation_unit(void);

static void
init_tokmap(void)
{
	TokMap[ TOK_UNKNOWN ] = 0;
	TokMap[ FLOATING_CONSTANT ] = TOK_CONSTANT|TOK_EXPR|TOK_STMT ;
	TokMap[ INTEGER_CONSTANT ] = TOK_CONSTANT|TOK_EXPR|TOK_STMT ;
	TokMap[ STRING_CONSTANT ] = TOK_CONSTANT|TOK_EXPR|TOK_STMT ;
	TokMap[ CHARACTER_CONSTANT ] = TOK_CONSTANT|TOK_EXPR|TOK_STMT ;
	TokMap[ IDENTIFIER ] = TOK_EXPR|TOK_STMT ;
	TokMap[ SIZEOF ] = TOK_EXPR|TOK_STMT ;
	TokMap[ AND ] = TOK_EXPR|TOK_STMT ;
	TokMap[ PLUSPLUS ] = TOK_EXPR|TOK_STMT ;
	TokMap[ MINUSMINUS ] = TOK_EXPR|TOK_STMT ;
	TokMap[ STAR ] = TOK_EXPR|TOK_STMT ;
	TokMap[ PLUS ] = TOK_EXPR|TOK_STMT ;
	TokMap[ MINUS ] = TOK_EXPR|TOK_STMT ;
	TokMap[ TILDE ] = TOK_EXPR|TOK_STMT ;
	TokMap[ LPAREN ] = TOK_EXPR|TOK_STMT ;
	TokMap[ NOT ] = TOK_EXPR|TOK_STMT ;
	TokMap[ TYPEDEF_NAME ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC;
	TokMap[ ICHAR ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ IFLOAT ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ IDOUBLE ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ ISHORT ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ IINT ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ UNSIGNED ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ SIGNED ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ IVOID ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ STRUCT ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_STRUCT|TOK_TAG|TOK_DECL_SPEC ;
	TokMap[ UNION ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_STRUCT|TOK_TAG|TOK_DECL_SPEC ;
	TokMap[ ENUM ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TAG|TOK_DECL_SPEC ;
	TokMap[ ILONG ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_SPECIFIER|TOK_DECL_SPEC ;
	TokMap[ ICONST ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_QUALIFIER|TOK_DECL_SPEC ;
	TokMap[ VOLATILE ] = TOK_TYPE_SPECIFIER_QUALIFIER|TOK_TYPE_QUALIFIER|TOK_DECL_SPEC ;
	TokMap[ STATIC ] = TOK_STORAGE_CLASS|TOK_DECL_SPEC ;
	TokMap[ EXTERN ] = TOK_STORAGE_CLASS|TOK_DECL_SPEC ;
	TokMap[ AUTO ] = TOK_STORAGE_CLASS|TOK_DECL_SPEC ;
	TokMap[ REGISTER ] = TOK_STORAGE_CLASS|TOK_DECL_SPEC ;
	TokMap[ TYPEDEF ] = TOK_STORAGE_CLASS|TOK_DECL_SPEC ;
	TokMap[ IF ] = TOK_STMT ;
	TokMap[ BREAK ] = TOK_STMT ;
	TokMap[ CASE ] = TOK_STMT ;
	TokMap[ CONTINUE ] = TOK_STMT ;
	TokMap[ DEFAULT ] = TOK_STMT ;
	TokMap[ DO ] = TOK_STMT ;
	TokMap[ ELSE ] = TOK_STMT ;
	TokMap[ FOR ] = TOK_STMT ;
	TokMap[ GOTO ] = TOK_STMT ;
	TokMap[ RETURN ] = TOK_STMT ;
	TokMap[ SWITCH ] = TOK_STMT ;
	TokMap[ WHILE ] = TOK_STMT ;
	TokMap[ LBRACE ] = TOK_STMT;
	TokMap[ SEMI ] = TOK_STMT; 
	TokMap[ EQUALS ] = TOK_ASSGNOP ;
	TokMap[ PLUS_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ MINUS_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ STAR_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ SLASH_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ PERCENT_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ LSHIFT_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ RSHIFT_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ AND_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ XOR_EQUALS ] = TOK_ASSGNOP ;
	TokMap[ OR_EQUALS ] = TOK_ASSGNOP ;

}

const char *
object_name(int object_type)
{
	const char *name;

	switch(object_type) {
	case OBJ_TYPEDEF_NAME: name = "typedef"; break;
	case OBJ_FUNCTION_DECL: name = "function_decl"; break;
	case OBJ_FUNCTION_DEFN: name = "function_defn"; break;
	case OBJ_ENUMERATOR: name = "enumerator"; break;
	case OBJ_VARIABLE: name = "variable"; break;
	case OBJ_PARAMETER: name = "parameter"; break;
	case OBJ_IDENTIFIER: name = "identifier"; break;
	case OBJ_TYPE: name = "extended type"; break;
	case OBJ_LABEL: name = "label"; break;
	default: name = "invalid"; break;
	}
	return name;
}
static unsigned int ComputeHash(const char *string)
{
    unsigned long hash = 5381;
    unsigned int c;

    while (c = *string++)
        hash = ((hash << 5) + hash) ^ c;

    return hash % HASH_SIZE;
}

symtab_t *
new_symbol_table(list_t *owner, bool hashed)
{
	symtab_t *tab;

	tab = allocate(sizeof(symtab_t));
	list_append(owner, tab);
	if (hashed)
	{
		int i;
		tab->hashed_symbols = allocate(HASH_SIZE * sizeof(list_t));
		for (i=0; i < HASH_SIZE; i++)
			list_init(&tab->hashed_symbols[i]);
	}
	else
	{
		list_init(&tab->symbols);
	}
	return tab;
}

static void
init_symbol_table(void)
{
	list_init(&identifiers);
	Cursymtab = new_symbol_table(&identifiers, TRUE);
	list_init(&labels);
	list_init(&types);
	Curtypes = new_symbol_table(&types, TRUE);
	list_init(&exited_scopes);
	list_init(&exited_types);
	
	list_init(&labels);
}

symbol_t *
find_symbol(symtab_t *tab, const char *name, int all_scope)
{
	symbol_t *sym;
	list_t *list = &tab->symbols;
	assert(tab != 0);
	if (tab->hashed_symbols)
	{
		unsigned hash = ComputeHash(name);
		list = &tab->hashed_symbols[hash];
	}
	for (sym = (symbol_t *)list_first(list);
		sym != 0;
		sym = (symbol_t *)list_next(list, sym)) {
		if (strcmp(sym->name, name) == 0)
			return sym;
	}

	if (!all_scope)
		return 0;
	if (all_scope == 1)
		tab = (symtab_t *)list_prev(&identifiers, tab);
	else
		tab = (symtab_t *)list_prev(&types, tab);
	if (!tab)
		return 0;
	return find_symbol(tab, name, all_scope);
}

static void
enter_scope(void)
{
	symtab_t *tab;

	if (Level == LEVEL_GLOBAL)
	{
		list_init(&labels);
		Curlabels = new_symbol_table(&labels, FALSE);
	}

	Level++;
	if (DebugLevel >= 3) { 
		xprint("%*sEntering scope %d\n", TraceLevel, "", Level); 
	}
	if (Level == LEVEL_STATEMENT) {
		/* Although we increment the level when we parse 
		 * function parameters, and again when we enter the
		 * function body, ANSI C requires these to be in the same
		 * scope.
		 */
		return;
	}
	/*
	tab = calloc(1, sizeof(symtab_t));
	list_init(&tab->symbols);
	list_append(&identifiers, tab);
	Cursymtab = tab;
	*/
	Cursymtab = new_symbol_table(&identifiers, FALSE);
	Curtypes = new_symbol_table(&types, FALSE);
}

static void
exit_scope(void)
{
	symtab_t *tab;
	symbol_t *sym;
	
	if (DebugLevel >= 3) { 
		xprint("%*sExiting scope %d\n", TraceLevel, "", Level); 
	}
	Level--;
    if (Level != LEVEL_GLOBAL)
    {
    	sym = list_first(&Cursymtab->symbols);
    	while (sym)
    	{
    		sym->endline = Lex_env->le_lnum;
    		sym = list_next(&Cursymtab->symbols, sym);
    	}
    	sym = list_first(&Curtypes->symbols);
    	while (sym)
    	{
    		sym->endline = Lex_env->le_lnum;
    		sym = list_next(&Curtypes->symbols, sym);
    	}
    }
	/* Although we increment the Level when we parse 
	 * function parameters, and again when we enter the
	 * function body, ANSI C requires these to be in the same
	 * scope.
	 */
	if (Level == LEVEL_FUNCTION)
		return;
	tab = Cursymtab;
	Cursymtab = (symtab_t *)list_prev(&identifiers, Cursymtab);
	assert(Cursymtab != 0);
	list_remove(&identifiers, tab);
	list_append(&exited_scopes, tab);
	
	tab = Curtypes;
	Curtypes = (symtab_t *)list_prev(&types, Curtypes);
	assert(Curtypes != 0);
	list_remove(&types, tab);
	list_append(&exited_types, tab);

	if (Level == LEVEL_GLOBAL)
	{
		// any post function cleanup goes here
	}
}

symbol_t *
install_symbol(symtab_t *tab, const char *name, Storageclass storage_class, int object_type)
{
	symbol_t *sym;
	list_t *list = &tab->symbols;
	sym = find_symbol(tab, name, 0);
	if (sym != 0) {
		xprint( "Error: redeclaration of symbol %s as %s\n", name,
			object_name(object_type));
		xprint( "Error: previously declared as %s\n", 
			object_name(sym->object_type));
		/* xprint( "Level = %d\n", Level); */
		/* exit(1); */
	}
	if (DebugLevel >= 3) {
		xprint("%*sInstalling %s name %s\n", TraceLevel, "", object_name(object_type), name);
		if (sym) {
			xprint("%*s\tOverriding %s name %s\n", TraceLevel, "", object_name(sym->object_type), sym->name);
		}
	}
	sym = allocate( sizeof(symbol_t));
	sym->name = string_copy(name, strlen(name));
	sym->storage_class = storage_class;
	sym->object_type = object_type;
	if (Lex_env)
	{
		sym->startline = sym->endline = Lex_env->le_lnum;
		sym->file = Lex_env->le_filename;
	}
	
	if (tab->hashed_symbols)
	{
		unsigned hash = ComputeHash(name);
		list = &tab->hashed_symbols[hash];
	}
	list_append(list, sym);
	lastInstalledSymbol = sym;
	return sym;
}	
static type_t *new_type(enum Basictype enumtype)
{
	type_t *type = allocate(sizeof(type_t));
	type->btype = enumtype;
	return type;
}
static void
match(token_t expected_tok)
{
	if (tok != expected_tok) {
		xprint("Parse failed: Expected %s, ", tokname(expected_tok));
		xprint("got %s\n", tokname(tok));
		xprint("file: %s, line %d\n", Lex_env->le_filename, Lex_env->le_lnum);
		/* xprint("Level = %d\n", Level); */
		tok = lex_get_token();
		return;
	}
	else {
		if (tok == BADTOK)
		{
			xprint("Parse failed: BAD TOKEN\n");
			xprint("file: %s, line %d\n", Lex_env->le_filename, Lex_env->le_lnum);
		}
		else if (DebugLevel >= 2) {
			const char *cp = 0;
			if (TokMap[tok] & TOK_CONSTANT) {
				cp = Lexeme->constant->co_val;
			}
			else if (tok == IDENTIFIER) {
				cp = Lexeme->identifier->id_name;
			}
			if (cp == 0)
				xprint("%*s[%s]\n", TraceLevel, "", tokname(tok)); 
			else
				xprint("%*s[%s(%s)]\n", TraceLevel, "", tokname(tok), cp); 
		}
		tok = lex_get_token();
	}
}

static bool
check_not_typedef(void)
{
	return (Cursym == 0 || Cursym->object_type != OBJ_TYPEDEF_NAME);
}

static void
constant_expression(void)
{
	TRACEIN("constant_expression");
	conditional_expression();
	/* fold constant */
	TRACEOUT("constant_expression");
}

static void
expression(void)
{
	TRACEIN("expression");

	if (!is_expression(tok)) {
		TRACEOUT("expression");
		return;
	}

	assignment_expression();
	while (tok == COMMA) {
		match(COMMA);
		assignment_expression();
	}
	TRACEOUT("expression");
}

static void
primary_expression(void)
{
	TRACEIN("primary_expression");
	if (tok == IDENTIFIER) {
		check_not_typedef();
		match(IDENTIFIER);
	}
	else if (TokMap[tok] & TOK_CONSTANT) {
		match(tok);
	}
	/* parenthesized expression handled in unary_expression() */
	TRACEOUT("primary_expression");
}

static void
postfix_operator(void)
{
	TRACEIN("postfix_operator");
	if (tok == LBRAC) {
		match(LBRAC);
		// in both c & c99 an empty [] is allowed if not in a structure
		// in C99, the empty [] is also allowed on the last element of a structure
		// in which case 1 element will be allocated.
		expression();
		match(RBRAC);
	}
	else if (tok == LPAREN) {
		match(LPAREN);
		if (tok != RPAREN) {
			assignment_expression();
			while (tok == COMMA) {
				match(COMMA);
				assignment_expression();
			}
		}
		match(RPAREN);
	}
	else if (tok == DOT || tok == ARROW) {
		match(tok);
		match(IDENTIFIER);
	}
	else if (tok == PLUSPLUS || tok == MINUSMINUS) {
		match(tok);
	}
	TRACEOUT("postfix_operator");
}

static void
postfix_operators(void)
{
	TRACEIN("postfix_operators");
	while (tok == LBRAC || tok == LPAREN || tok == DOT ||
		tok == ARROW || tok == PLUSPLUS || tok == MINUSMINUS) {
		postfix_operator();
	}
	TRACEOUT("postfix_operators");
}

static void
sizeof_expression(void)
{
	TRACEIN("sizeof_expression");
	
	match(SIZEOF);
	if (tok == LPAREN) {
		int found_typename = 0;
		match(LPAREN);
		if (is_type_name(tok)) {
			type_name();
			found_typename = 1;
		}
		else {
			expression();
		}
		match(RPAREN);
#if 1 /* as per comp.std.c */
		if (found_typename && tok == LBRACE) {
			initializer(0);
			postfix_operators();
		}
#endif
		else if (!found_typename) {
			postfix_operators();
		}
	}
	else {
		unary_expression();
	}
	TRACEOUT("sizeof_expression");
}

static void
unary_expression(void)
{
	TRACEIN("unary_expression");
	if (tok == SIZEOF) {
		sizeof_expression();
	}
	else if (tok == LPAREN) {
		int found_typename = 0;
		match(LPAREN);
		if (is_type_name(tok)) {
			type_name();
			found_typename = 1;
		}
		else {
			expression();
		}
		match(RPAREN);
		if (prm_c99 && found_typename && tok == LBRACE) {
			initializer(0);
			postfix_operators();
		}
		else if (!found_typename) {
			postfix_operators();
		}
		else {
			unary_expression();
		}
	}
	else if (tok == PLUSPLUS || tok == MINUSMINUS || tok == AND
		|| tok == STAR || tok == PLUS || tok == MINUS
		|| tok == TILDE || tok == NOT) {
		match(tok);
		unary_expression();
	}
	else {
		primary_expression();
		postfix_operators();
	}
	TRACEOUT("unary_expression");
}

static void
multiplicative_expression(void)
{
	TRACEIN("multiplicative_expression");
	unary_expression();
	while (tok == STAR || tok == SLASH || tok == PERCENT) {
		match(tok);
		unary_expression();
	}
	TRACEOUT("multiplicative_expression");
}

static void
additive_expression(void)
{
	TRACEIN("additive_expression");
	multiplicative_expression();
	while (tok == PLUS || tok == MINUS) {
		match(tok);
		multiplicative_expression();
	}
	TRACEOUT("additive_expression");
}

static void
shift_expression(void)
{
	TRACEIN("shift_expression");
	additive_expression();
	while (tok == LSHIFT || tok == RSHIFT) {
		match(tok);
		additive_expression();
	}
	TRACEOUT("shift_expression");
}

static void
relational_expression(void)
{
	TRACEIN("relational_expression");
	shift_expression();
	while (tok == GREATERTHAN || tok == LESSTHAN || tok == GTEQ || tok == LESSEQ) {
		match(tok);
		shift_expression();
	}
	TRACEOUT("relational_expression");
}

static void
equality_expression(void)
{
	TRACEIN("equality_expression");
	relational_expression();
	while (tok == EQEQ || tok == NOTEQ) {
		match(tok);
		relational_expression();
	}
	TRACEOUT("equality_expression");
}

static void
and_expression(void)
{
	TRACEIN("and_expression");
	equality_expression();
	while (tok == AND) {
		match(AND);
		equality_expression();
	}
	TRACEOUT("and_expression");
}

static void
exclusive_or_expression(void)
{
	TRACEIN("exclusive_or_expression");
	and_expression();
	while (tok == XOR) {
		match(XOR);
		and_expression();
	}
	TRACEOUT("exclusive_or_expression");
}

static void
inclusive_or_expression(void)
{
	TRACEIN("inclusive_or_expression");
	exclusive_or_expression();
	while (tok == OR) {
		match(OR);
		exclusive_or_expression();
	}
	TRACEOUT("inclusive_or_expression");
}

static void
logical_and_expression(void)
{
	TRACEIN("logical_and_expression");
	inclusive_or_expression();
	while (tok == ANDAND) {
		match(ANDAND);
		inclusive_or_expression();
	}
	TRACEOUT("logical_and_expression");
}


static void
logical_or_expression(void)
{
	TRACEIN("logical_or_expression");
	logical_and_expression();
	while (tok == OROR) {
		match(OROR);
		logical_and_expression();
	}
	TRACEOUT("logical_or_expression");
}

static void
conditional_expression(void)
{
	TRACEIN("conditional_expression");
	logical_or_expression();
	if (tok == QUERY) {
		match(QUERY);
		expression();
		match(COLON);
		conditional_expression();
	}
	TRACEOUT("conditional_expression");
}

static void
assignment_expression(void)
{
	TRACEIN("assignment_expression");
	conditional_expression();
	if (is_assign_operator(tok)) {
		/* TODO: check that previous expression was unary */
		match(tok);
		assignment_expression();
	}
	TRACEOUT("assignment_expression");
}

static void
labeled_statement(void)
{
	TRACEIN("labeled_statement");
	match(IDENTIFIER);
	match(COLON);
	install_symbol(Curlabels, Lexeme->identifier->id_name, 
		SC_LABEL, OBJ_LABEL);
	statement();
	TRACEOUT("labeled_statement");
}

static void
case_statement(void)
{
	TRACEIN("case_statement");
	match(CASE);
	constant_expression();
	match(COLON);
	statement();
	TRACEOUT("case_statement");
}

static void
default_statement(void)
{
	TRACEIN("default_statement");
	match(DEFAULT);
	match(COLON);
	statement();
	TRACEOUT("default_statement");
}

static void
if_statement(void)
{
	TRACEIN("if_statement");
	enter_scope();
	match(IF);
	match(LPAREN);
	expression();
	match(RPAREN);
	enter_scope();
	statement();
	exit_scope();
	if (tok == ELSE) {
		enter_scope();
		match(ELSE);
		statement();
		exit_scope();
	}
	exit_scope();
	TRACEOUT("if_statement");
}

static void
switch_statement(void)
{
	TRACEIN("switch_statement");
	enter_scope();
	match(SWITCH);
	match(LPAREN);
	expression();
	match(RPAREN);
	enter_scope();
	statement();
	exit_scope();
	exit_scope();
	TRACEOUT("switch_statement");
}

static void
while_statement(void)
{
	TRACEIN("while_statement");
	enter_scope();
	match(WHILE);
	match(LPAREN);
	expression();
	match(RPAREN);
	enter_scope();
	statement();
	exit_scope();
	exit_scope();
	TRACEOUT("while_statement");
}

static void
do_while_statement(void)
{
	TRACEIN("do_while_statement");
	enter_scope();
	match(DO);
	enter_scope();
	statement();
	exit_scope();
	match(WHILE);
	match(LPAREN);
	expression();
	match(RPAREN);
	exit_scope();
	match(SEMI);
	TRACEOUT("do_while_statement");
}

static void
for_statement(void)
{
	TRACEIN("for_statement");
	enter_scope();
	match(FOR);
	match(LPAREN);
	if (tok != SEMI) {
		if (prm_c99 && is_declaration(tok)) {
			declaration();
		}
		else {	
			expression();
			match(SEMI);
		}
	}
	else {
		match(SEMI);
	}
	if (tok != SEMI)
		expression();
	match(SEMI);
	if (tok != RPAREN)
		expression();
	match(RPAREN);
	enter_scope();
	statement();
	exit_scope();
	exit_scope();
	TRACEOUT("for_statement");
}

static void
break_statement(void)
{
	TRACEIN("break_statement");
	match(BREAK);
	match(SEMI);
	TRACEOUT("break_statement");
}

static void
continue_statement(void)
{
	TRACEIN("continue_statement");
	match(CONTINUE);
	match(SEMI);
	TRACEOUT("continue_statement");
}

static void
goto_statement(void)
{
	TRACEIN("goto_statement");
	match(GOTO);
	match(IDENTIFIER);
	match(SEMI);
	TRACEOUT("goto_statement");
}

static void
return_statement(void)
{
	TRACEIN("return_statement");
	match(RETURN);
	if (tok != SEMI)
		expression();
	match(SEMI);
	TRACEOUT("return_statement");
}

static void
empty_statement(void)
{
	TRACEIN("empty_statement");
	match(SEMI);
	TRACEOUT("empty_statement");
}

static void
expression_statement(void)
{
	TRACEIN("expression_statement");

	if (tok == IDENTIFIER && lex_colon_follows()) {
		labeled_statement();
	}
	else {
		expression();
		match(SEMI);
	}
	TRACEOUT("expression_statement");
}

static void
statement(void)
{
	TRACEIN("statement");
	switch (tok) {
	case IDENTIFIER: expression_statement(); break;
	case CASE: case_statement(); break;
	case DEFAULT: default_statement(); break;
	case IF: if_statement(); break;
	case SWITCH: switch_statement(); break;
	case WHILE: while_statement(); break;
	case DO: do_while_statement(); break;
	case FOR: for_statement(); break;
	case BREAK: break_statement(); break;
	case CONTINUE: continue_statement(); break;
	case GOTO: goto_statement(); break;
	case RETURN: return_statement(); break;
	case LBRACE: compound_statement(); break;
	case SEMI: empty_statement(); break;
	default: 
		if (is_expression(tok))
			expression_statement(); 
		else
		{
			// error
			match(SEMI);
		}
		break;
	}
	TRACEOUT("statement");
}


static void
compound_statement(void)
{
	bool first = TRUE;
	TRACEIN("compound_statement");
	enter_scope();
	match(LBRACE);

	while (tok != RBRACE && tok != TOK_UNKNOWN) {
		if ((prm_c99 || first) && is_declaration(tok)) {
			if (tok == IDENTIFIER && lex_colon_follows())
			{
				statement();
				first = FALSE;
			}
			else
				declaration();
		}
		else {
			first = FALSE;
			statement();
		}
	}
	exit_scope();
	// if returning from main() at global level, C99 needs a 0 when no ret stmt
	// at end of block
	match(RBRACE);
	TRACEOUT("compound_statement");
}

static void
enumerator(type_t *type)
{
	TRACEIN("enumerator");
	if (tok == IDENTIFIER) {
		type_t *inttype = new_type(BT_INT);
		symbol_t *sym;
		check_not_typedef();
		sym = install_symbol(Cursymtab, Lexeme->identifier->id_name, 
			SC_CONSTANT, OBJ_ENUMERATOR);
		sym->type = inttype;
		if (type)
		{
			sym = install_symbol(type->symbols, Lexeme->identifier->id_name, 
				SC_CONSTANT, OBJ_ENUMERATOR);
			sym->type = inttype;
		}
		match(IDENTIFIER);
	}
	else {
		TRACEOUT("enumerator");
		return;
	}
	if (tok == EQUALS) {
		match(EQUALS);
		constant_expression();
	}
	TRACEOUT("enumerator");
}

static type_t *
enum_specifier(void)
{
	symbol_t *sym = NULL;
	type_t *type = NULL;
	TRACEIN("enum_specifier");
	if (tok == ENUM) {
		match(ENUM);
	}
	else {
		TRACEOUT("enum_specifier");
		return;
	}
	if (tok == IDENTIFIER) {
		if (!(sym = find_symbol(Curtypes, Lexeme->identifier->id_name, 4)))
		{
			sym = install_symbol(Curtypes, Lexeme->identifier->id_name, SC_TYPE, OBJ_TYPE);
		}
		else
			type = sym->type;
		match(IDENTIFIER);
	}
	else
	{
		char buf[256];
		sprintf(buf,"###%d",uid++);
		sym = install_symbol(Curtypes, buf, SC_TYPE, OBJ_TYPE);
	}
	if (type == NULL)
	{
		type = new_type(BT_ENUM);
		sym->type = type;
		type->name = sym->name;
	}
	if (tok == LBRACE) {
		match(LBRACE);
		type->symbols = allocate( sizeof(list_t));
		list_init(type->symbols);
		type->symbols = new_symbol_table(type->symbols,FALSE);		
		enumerator(type);
		while (tok == COMMA) {
			match(COMMA);
			if (!prm_c99 && tok != IDENTIFIER)
			{
				// error
			}
			enumerator(type);
		}
		match(RBRACE);
	}
	TRACEOUT("enum_specifier");
	return type;
}

static void
member(type_t *type)
{
	TRACEIN("member");
	if (tok != COLON)
		declarator(type, 0);
	if (tok == COLON) {
		match(COLON);
		constant_expression();
	}
	TRACEOUT("member");
}

static void
members(void)
{
	type_t *type;
	TRACEIN("members");
	do {
		stack_ptr++;
		type = declaration_specifiers(1);
		member(type);
		while (tok == COMMA) {
			match(COMMA);
			member(type);
		}
		match(SEMI);
		stack_ptr--;
	} while (tok != RBRACE && tok != TOK_UNKNOWN);
	TRACEOUT("members");
}

static type_t *
struct_or_union_specifier(bool isunion)
{
	symbol_t *sym = NULL;
	type_t *type = NULL;
	TRACEIN("struct_or_union_specifier");
	Parsing_struct++;
	match(tok);
	if (tok == IDENTIFIER)
	{
		if (!(sym = find_symbol(Curtypes, Lexeme->identifier->id_name, 4)))
		{
			sym = install_symbol(Curtypes, Lexeme->identifier->id_name, SC_TYPE, OBJ_TYPE);
		}
		else
			type = sym->type;
		match(IDENTIFIER);
	}
	else
	{
		char buf[256];
		sprintf(buf,"###%d",uid++);
		sym = install_symbol(Curtypes, buf, SC_TYPE, OBJ_TYPE);
	}
	if (type == NULL)
	{
		type = new_type(isunion ? BT_UNION : BT_STRUCT);
		sym->type = type;
		type->name = sym->name;
	}
	if (tok == LBRACE) {
		symtab_t *current = Curstruct;
		match(LBRACE);

		type->symbols = allocate( sizeof(list_t));
		list_init(type->symbols);
		type->symbols = new_symbol_table(type->symbols,FALSE);		

		sym->data = allocate( sizeof(list_t));
		Curstruct = type->symbols;
		members();
		Curstruct = current;
		match(RBRACE);
	}
	Parsing_struct--;
	TRACEOUT("struct_or_union_specifier");
	return type;
}

static void
type_name(void)
{
	type_t *type ;
	TRACEIN("type_name");
	stack_ptr++;
	type = declaration_specifiers(1);
	declarator(type, 1);
	stack_ptr--;
	TRACEOUT("type_name");
}

static Storageclass translate_storage_class(token_t tok)
{
	switch(tok)
	{
		case STATIC:
			return SC_STATIC;
		case EXTERN:
			return SC_EXTERNAL;
		case REGISTER:		
		case AUTO:
			return SC_AUTO;
		case TYPEDEF:
			return SC_TYPE;
		default:
			xprint( "unknown storage class");
			return SC_STATIC;
					
	}
}
#define ILONGLONG (31)
static type_t *translate_type(int btype)
{
	type_t *type = new_type(BT_MATCHNONE);
	switch(btype & ~((1 << ICONST) | (1 << VOLATILE) | (1 << RESTRICT)))
	{
		case (1 << IBOOLEAN):
			type->btype == BT_BOOL;
			break;
		case (1 << COMPLEX) | (1 << IFLOAT):
			type->btype = BT_FLOATCOMPLEX;
			break;
		case (1 << COMPLEX) | (1 << IDOUBLE):
			type->btype = BT_DOUBLECOMPLEX;
			break;
		case (1 << COMPLEX) | (1 << IDOUBLE) | (1 << ILONG):
			type->btype = BT_LONGDOUBLECOMPLEX;
			break;
		case (1 << IMAGINARY) | (1 << IFLOAT):
			type->btype = BT_FLOATIMAGINARY;
			break;
		case (1 << IMAGINARY) | (1 << IDOUBLE):
			type->btype = BT_DOUBLEIMAGINARY;
			break;
		case (1 << IMAGINARY) | (1 << IDOUBLE) | (1 << ILONG):
			type->btype = BT_LONGDOUBLEIMAGINARY;
			break;
		case (1 << IFLOAT):
			type->btype = BT_FLOAT;
			break;
		case (1 << IDOUBLE):
			type->btype = BT_DOUBLE;
			break;
		case (1 << IDOUBLE) | (1 << ILONG):
			type->btype = BT_LONGDOUBLE;
			break;
		case (1 << ICHAR):
			type->btype = BT_CHAR;
			break;
		case (1 << ICHAR) | (1 << SIGNED):
			type->btype = BT_CHAR;
			break;
		case (1 << ICHAR) | (1 << UNSIGNED):
			type->btype = BT_UNSIGNEDCHAR;
			break;
		case (1 << ISHORT):
			type->btype = BT_SHORT;
			break;
		case (1 << ISHORT) | (1 << SIGNED):
			type->btype = BT_SHORT;
			break;
		case (1 << ISHORT) | (1 << UNSIGNED):
			type->btype = BT_UNSIGNEDSHORT;
			break;
		case (1 << ISHORT) | (1 << SIGNED) | (1 << IINT):
			type->btype = BT_SHORT;
			break;
		case (1 << ISHORT) | (1 << UNSIGNED) | (1 << IINT):
			type->btype = BT_UNSIGNEDSHORT;
			break;
		case (1 << IINT):
			type->btype = BT_INT;
			break;
		case (1 << IINT) | (1 << SIGNED):
		case (1 << SIGNED):
			type->btype = BT_INT;
			break;
		case (1 << IINT) | (1 << UNSIGNED):
		case (1 << UNSIGNED):
			type->btype = BT_UNSIGNEDINT;
			break;
		case (1 << ILONG):
			type->btype = BT_LONG;
			break;
		case (1 << ILONG) | (1 << SIGNED):
			type->btype = BT_LONG;
			break;
		case (1 << ILONG) | (1 << UNSIGNED):
			type->btype = BT_UNSIGNEDLONG;
			break;
		case (1 << ILONG) | (1 << SIGNED) | (1 << IINT):
			type->btype = BT_LONG;
			break;
		case (1 << ILONG) | (1 << UNSIGNED) | (1 << IINT):
			type->btype = BT_UNSIGNEDLONG;
			break;
		case (1 << ILONGLONG):
			type->btype = BT_LONGLONG;
			break;
		case (1 << ILONGLONG) | (1 << SIGNED):
			type->btype = BT_LONGLONG;
			break;
		case (1 << ILONGLONG) | (1 << UNSIGNED):
			type->btype = BT_UNSIGNEDLONGLONG;
			break;
		case (1 << ILONGLONG) | (1 << SIGNED) | (1 << IINT):
			type->btype = BT_LONGLONG;
			break;
		case (1 << ILONGLONG) | (1 << UNSIGNED) | (1 << IINT):
			type->btype = BT_UNSIGNEDLONGLONG;
			break;
		case (1 << IVOID):
			type->btype = BT_VOID;
			break;
		case 0:
			type->btype = BT_INT;
			break;
		default:
			xprint( "invalid type specification");
			break;
	}
	return type;
}
void translate_qualifiers(type_t * type, int btype)
{
	if (type)
	{
		if (btype & (1 << ICONST))
			type->t_const = TRUE;
		if (btype & (1 << VOLATILE))
			type->t_volatile = TRUE;
		if (btype & (1 << RESTRICT))
			type->t_restrict = TRUE;
		// ignoring register keyword
	}
}
static type_t *
declaration_specifiers(int no_storage_class)
{
	type_t *type = NULL;
	symbol_t *sym = NULL;
	int btype = 0;
	bool type_found = FALSE;
	TRACEIN("declaration_specifiers");
	
	assert(stack_ptr >= 0 && stack_ptr < sizeof(Storage_class)/sizeof(Storage_class[0]));
	Storage_class[stack_ptr] = SC_GLOBAL;
	if (Level != LEVEL_GLOBAL) 
		Storage_class[stack_ptr] = SC_AUTO;
	if (Parsing_struct)
		Storage_class[stack_ptr] = SC_MEMBER;
	while (is_declaration(tok)) {
		if (no_storage_class && (TokMap[tok] & TOK_STORAGE_CLASS)) {
			xprint( "Parse failed: unexpected storage class %s\n", tokname(tok));
			xprint( "file: %s, line %d", Lex_env->le_filename, Lex_env->le_lnum);
// should just be an error
//			exit(1);
		}
		if (TokMap[tok] & TOK_STRUCT) {
			type = struct_or_union_specifier(tok == UNION);
			type_found = TRUE;
			break;
		}
		else if (tok == ENUM) {
			type = enum_specifier();
			type_found = TRUE;
			break;
		}
		else if (TokMap[tok] & TOK_TYPE_SPECIFIER_QUALIFIER)
		{
			if (tok == ILONG)
			{
				if (btype & (1 << ILONG))
				{
					btype &= ~(1 << ILONG);
					btype |= (1 << ILONGLONG);
				}
				else
					btype |= (1 << (int)tok);
			}
			else
				btype |= (1 << (int)tok);
			type_found = TRUE;
		}
		else if (tok == IDENTIFIER) {
			symbol_t *sym1 = find_symbol(Cursymtab, Lexeme->identifier->id_name, TRUE);
			if (sym1 && (sym1->object_type & OBJ_TYPEDEF_NAME))
			{
				type = sym1->type;
				type_found = TRUE;
				
			}
			else
				break;
		}
		else if (TokMap[tok] & TOK_STORAGE_CLASS) {
			Storage_class[stack_ptr] = translate_storage_class(tok);
		} 
		match(tok);
	}
	if (type_found)
	{
		if (type == NULL)
			type = translate_type(btype);
	}
	else
		type = translate_type(1 << IINT); // C89 compatibility
	translate_qualifiers(type, btype);
	
	if (!prm_c99 && (type->btype == BT_LONGLONG || type->btype == BT_UNSIGNEDLONGLONG))
	{
		// error - invalid type
	}
	TRACEOUT("declaration_specifiers");
	return type;
}

static type_t *
prepend_ptr(type_t *type, bool byValue)
{
	type_t *ntype = new_type(BT_POINTER);
	if (!type)
		return 0;
	ntype->link = type;
	ntype->byValue = byValue;
	return ntype;
}
static type_t *
pointer(type_t *type, bool byValue)
{
	TRACEIN("pointer");
	while (tok == STAR) {
		type = prepend_ptr(type, FALSE);
		match(STAR);
		while (TokMap[tok] & TOK_TYPE_QUALIFIER) {
			translate_qualifiers(type, (1 << tok));
			match(tok);
		}
	}
	TRACEOUT("pointer");
	return type;
}

static type_t *
direct_declarator(type_t *type, int abstract)
{
	symbol_t *rsym = NULL;
	char *name;
	TRACEIN("direct_declarator");
	if (tok == LPAREN) {
		/* the extra complexity is to deal with things like void (*a)()
		 * or int (*a)[]
		 * where the '*' in this case makes the whole declaration a pointer
		 * rather than tacking the suffix on front
		 */
		type_t *ntype = NULL, *ptype;
		match(LPAREN);
		ntype = declarator(ntype, abstract);
		match(RPAREN);
		/* this is going to be done again when we return from this function,
		 * however this exhausts the token stream so the next run won't do
		 * anything
		 */
		while (tok == LBRAC || tok == LPAREN) {
			suffix_declarator();
		}
		/* put the parenthesized type in front */
		if (ntype)
		{
			ptype = ntype;
			while (ptype->link)
				ptype = ptype->link;
			ptype->link = type;
			lastInstalledSymbol->type = ptype;
			type = ntype;
		}
	}
	else {
		/* in here we *may* have to create a dummy symbol
		 * if none was declared... e.g. for function args.
		 * otherwise lack of symbol is probably an error
		 */
		if (!abstract) {
            if (tok == IDENTIFIER)
                Saw_ident = 1;
//			if (tok == IDENTIFIER) {
            {
	if (Lexeme->identifier)
	  name = Lexeme->identifier->id_name ;
                else
                  name = "???";
				if (Storage_class[stack_ptr] == SC_TYPE) {
					rsym = install_symbol(Cursymtab, name, 
						SC_TYPE, OBJ_TYPEDEF_NAME);
					
					if (type)
					{
						if (type->typedefname)
						{
							type_t *type1 = new_type(type->btype);
							*type1 = *type;
							type = type1;
						}
						type->typedefname = rsym->name;
					}
					rsym->type = type;
				}
				else if (!Parsing_struct && !Parsing_oldstyle_parmdecl) {
					rsym = install_symbol(Cursymtab, name, 
						Storage_class[stack_ptr], OBJ_IDENTIFIER);
					rsym->type = type;
				}
				else if (Parsing_struct)
				{
					rsym = install_symbol(Curstruct, name, 
						Storage_class[stack_ptr], OBJ_IDENTIFIER);
					rsym->type = type;
				}
                if (tok == IDENTIFIER)
    				match(IDENTIFIER);
			}
		}
	}
	TRACEOUT("direct_declarator");
	return type;
}

static void 
transfer_parameters(symtab_t *prototype)
{
	symtab_t *symtab = Cursymtab;
	symbol_t *sym = list_first(&symtab->symbols);
	while (sym)
	{
		symbol_t * nsym = install_symbol(prototype, sym->name,
			SC_AUTO, OBJ_PARAMETER);
		nsym->type = sym->type;
		sym = list_next(&symtab->symbols, sym);
	}
}
static void 
parameter_list(symtab_t *prototype, int *new_style)
{
	symbol_t *sym = lastInstalledSymbol;
	TRACEIN("parameter_list");
	if (tok == IDENTIFIER && (Cursym == 0 || Cursym->object_type != OBJ_TYPEDEF_NAME)) {
		*new_style = 0;
		install_symbol(Cursymtab, Lexeme->identifier->id_name, 
			SC_AUTO, OBJ_PARAMETER);
		install_symbol(prototype, Lexeme->identifier->id_name,
			SC_AUTO, OBJ_PARAMETER);
		match(IDENTIFIER);
		while (tok == COMMA) {
			match(COMMA);
			if (tok == IDENTIFIER) {
				check_not_typedef();
				install_symbol(Cursymtab, Lexeme->identifier->id_name, 
					SC_AUTO, OBJ_PARAMETER);
				install_symbol(prototype, Lexeme->identifier->id_name,
					SC_AUTO, OBJ_PARAMETER);
				match(tok);
			}
			else
				match(IDENTIFIER);
		}
	}
	else {
		type_t *type;
		/*
		 * CHECK: When defining a function, each declarator in a
		 * parameter list must contain an identifier.
		 */

		*new_style = 1;
		stack_ptr++;
		type = declaration_specifiers(0);
		declarator(type, 0);
		stack_ptr--;
		while (tok == COMMA) {
			match(COMMA);
			if (tok == ELLIPSIS) {
				match(ELLIPSIS);
				break;
			}
			stack_ptr++;
			type = declaration_specifiers(0);
			declarator(type, 0);
			stack_ptr--;
		}
		transfer_parameters(prototype);
	}
	TRACEOUT("parameter_list");
	lastInstalledSymbol = sym;
}

static void
suffix_declarator(void)
{
	type_t * type = lastInstalledSymbol->type;
	TRACEIN("suffix_declarator");
	if (tok == LBRAC) {
		type_t * root = NULL;
		while (tok == LBRAC)
		{
			match(LBRAC);
			if (root == NULL)
			{
				type = prepend_ptr(type, TRUE);
				root = type;
			}
			else
			{
				root->link = prepend_ptr(root->link, TRUE);
				root = root->link;
			}
			constant_expression(); // c99 could be an identifier...
			lastInstalledSymbol->type = type;
			match(RBRAC);
		}
	}
	else if (tok == LPAREN) {
		int new_style = 0;
		type_t *functype = new_type(BT_FUNC);
		functype->link = type;
		functype->symbols = allocate(sizeof(list_t));
		lastInstalledSymbol->type = functype;
		list_init(functype->symbols);
		functype->symbols = new_symbol_table(functype->symbols,FALSE);		
		enter_scope();
		match(LPAREN);
		parameter_list(functype->symbols, &new_style);
		match(RPAREN);
		if (new_style && tok != LBRACE)
			exit_scope();
		Is_func = 1;
	}
	TRACEOUT("suffix_declarator");
}


static type_t *
declarator(type_t *type, int abstract)
{
	TRACEIN("declarator");
	while (TokMap[tok] & TOK_TYPE_QUALIFIER) {
		translate_qualifiers(type, (1 << tok));
		match(tok);
	}
	if (tok == STAR) {
		type = pointer(type, FALSE);
	}
	direct_declarator(type, abstract);
	while (tok == LBRAC || tok == LPAREN) {
		suffix_declarator();
	}
TRACEOUT("declarator");
	return type;
}

static void
designator(void)
{
	TRACEIN("designator");
	if (tok == LBRAC) {
		match(LBRAC);
		constant_expression();
		match(RBRAC);
	}
	else if (tok == DOT) {
		match(DOT);
		if (tok == IDENTIFIER) {
			check_not_typedef();
			match(tok);
		}
	}
	TRACEOUT("designator");
}
	

static void
initializer(int recurse)
{
	TRACEIN("initializer");
	// can only do this at function level in C99
	if (tok == LBRACE) {
		match(LBRACE);
		initializer(recurse+1);
		while (tok == COMMA) {
			match(COMMA);
			initializer(recurse+1);
		}
		match(RBRACE);
	}
	else if (prm_c99 && recurse && (tok == LBRAC || tok == DOT)) {
		while (tok == LBRAC || tok == DOT) {
			designator();
		}
		match(EQUALS);
		initializer(0);
	}
	else {
		assignment_expression();
	}
	TRACEOUT("initializer");
}

static void
function_definition(void)
{
	TRACEIN("function_definition");

	if (tok == LBRACE) {
		compound_statement();
	}
	else {
		Parsing_oldstyle_parmdecl++;
		while (is_declaration(tok)) {
			/*
			* CHECK: The only storage class permitted is
			* register and initialization is not permitted.
			* if no declaration is given for a parameter,
			* its type is taken to be int.
			*/
	
		 	declaration();
	 	}
		Parsing_oldstyle_parmdecl--;
	 	compound_statement();
 	}
	exit_scope();
	TRACEOUT("function_definition");
}

static int
init_declarator(type_t *type, int check_if_function)
{
	int old_Is_func, old_Saw_ident;
	int func_defn = 0;
	TRACEIN("init_declarator");

	old_Saw_ident = Saw_ident;
	old_Is_func = Is_func;

	Saw_ident = 0;
	Is_func = 0;
	declarator(type, 0);

	func_defn = check_if_function &&
		Level == LEVEL_FUNCTION &&
		Is_func && 
		Saw_ident &&
		is_function_body(tok);

	if (Is_func) {
		/*
		 * CHECK: The only storage class specifiers allowed among the
		 * declaration specifiers are extern or static.
		 */

		/*
		 * CHECK: A function may not return a function or an array.
		 */
	}

	Is_func = old_Is_func;
	Saw_ident = old_Saw_ident;
	if (func_defn) {
		function_definition();
		TRACEOUT("init_declarator");
		return 1;
	}
	else {
		if (tok == EQUALS) {
			/*
		 	* CHECK: not allowed when parsing old style function parameters
		 	* or a prototype.
		 	*/
			match(EQUALS);
			initializer(0);
		}
	}
	TRACEOUT("init_declarator");
	return 0;
}

/*
 * 3) a declaration must have at least one declarator, or its type specifier
 * must declare a structure tag, a union tag, or the members of an enumeration.
 * 4) empty declarations are not permitted.
 */
static void
declaration(void)
{
	type_t *type;
	TRACEIN("declaration");
		
	stack_ptr++;
	type = declaration_specifiers(0);
	if ( tok == SEMI ) {
		match(SEMI);
		goto success;
	}
	/* 2) the first declarator at global level may start a function
	 * definition.
	 */
	if (init_declarator(type, Level == LEVEL_GLOBAL) == 1) {
		goto success;
	}
	while ( tok == COMMA ) {
		match(COMMA);
		init_declarator(type, 0);
	}
	match(SEMI);
success:
	stack_ptr--;
	TRACEOUT("declaration");
}

/*
 * 1) translation unit consists of a sequence of external declarations.
 * which are either declarations or function definitions.
 * 2) only at this level can functions be defined.
 */
static void
translation_unit(void)
{
	TRACEIN("translation_unit");
	Level = LEVEL_GLOBAL;
	tok = lex_get_token();
	while (tok != TOK_UNKNOWN && tok != BADTOK) {
		if (is_external_declaration(tok) || tok == IDENTIFIER && (prm_loose || !prm_c99)) {
			/* 
			 * a function definition looks like a declaration,
			 * hence is initially parsed as one.
			 * the check for 2) is in init_declarator().
			 */
			declaration();
		}
		else if (tok == SEMI) {
			/*ERROR(4): empty declarations are not permitted */
			match(tok);
		}
		else {
			xprint( "Parse failed: unexpected input %s\n",
				tokname(tok));
			xprint( "file: %s, line %d", Lex_env->le_filename, Lex_env->le_lnum);
			match(tok);
		}
		while (Level != LEVEL_GLOBAL)
			exit_scope();
	}
	TRACEOUT("translation_unit");
}

token_t
name_type(const char *name)
{
	Cursym = find_symbol(Cursymtab, name, 1);
	return IDENTIFIER;
}

static const char *
mygetline(char *arg)
{
	static char line[512];
	line[0] = 0;

	return fgets(line, sizeof line, (FILE *)arg);
}

void parser_main(char *name)
{
	int i;
	symtab_t *tab;
	lex_env_t mylex = {0};
    FILE *fp;
	const char *cp = getenv("DEBUG");

	if (cp != 0) {
		DebugLevel = atoi(cp);
	}

 	Lex_env = &mylex;
	Lex_env->le_filename = allocate(_MAX_PATH);
	strcpy(Lex_env->le_filename, name);
//	Lex_env->le_getline = string_getline;
//	Lex_env->le_getline_arg = (char *)&curptr;
	Lex_env->ifskip = allocate(sizeof(ifskip_t));
	if (get_include_context(Lex_env, FALSE))
		return;
	Lexeme = &Lex_env->le_lexeme;

    init_tokmap();
	init_symbol_table();
#if 0
        putenv("LEX_DEBUG=1");
#endif
	translation_unit();
	
//	free_include_context(Lex_env);
	
	tab = list_first(&identifiers);
	for (i=0; i < HASH_SIZE; i++)
	{
		list_t *list = &tab->hashed_symbols[i];
		symbol_t *sym = list_first(list);
		while (sym)
		{
			if (strcmp(sym->file, Lex_env->le_filename) != 0)
				sym->startline = 1;
			sym->endline = Lex_env->le_lnum;
			sym = list_next(list, sym);
		}
	}
	free_include_context(Lex_env);
	dump();
	freeall();
        return;
}

