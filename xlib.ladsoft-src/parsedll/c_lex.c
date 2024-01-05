/* c_lex.c - a standalone C lexical analyser */

/*  Copyright 1991 Mark Russell, University of Kent at Canterbury.
 *
 *  You can do what you like with this source code as long as
 *  you don't try to make money out of it and you include an
 *  unaltered copy of this message (including the copyright).
 */

/*
 *  This standalone version created on Jan 19 1998.
 *  Author : Dibyendu Majumdar
 *  Email  : dibyendu@mazumdar.demon.co.uk  
 *  Website: www.mazumdar.demon.co.uk
 */

/*
 * 21 Jan 2001 Added support for inline, restrict, _Bool, _Complex, and _Imaginary.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include "c_lex.h"
#include "c_list.h"
#include "c_parser.h"
#include "c_preprc.h"
#include "c_alloc.h"

void * userAlloc(int size);
void userFree(void *ptr);

static int commentlevel;
static bool Want_debugging_output;

/* static const char *tokname (token_t token); */
static const char *skip_whitespace (lex_env_t *le, const char *line);
static int get_float_constant (lex_env_t *le, const char *line,
					const char **p_end, constant_t *co);
static const char *getline (lex_env_t *le);
static int get_string (lex_env_t *le, const char *line, constant_t *co);

static struct {
	const char *name;
	token_t token;
	bool need_lexinfo;
	bool C99;
} Keytab[] = {
	{"_Bool",	IBOOLEAN,		FALSE,	TRUE},
	{"_Complex",	COMPLEX,	FALSE,	TRUE},
	{"_Imaginary",	IMAGINARY,	FALSE,	TRUE},
	{"auto",	AUTO,		FALSE,	FALSE},
	{"break",	BREAK,		TRUE,	FALSE},
	{"case",	CASE,		FALSE,	FALSE},
	{"char",	ICHAR,		FALSE,	FALSE},
	{"const",	ICONST,		FALSE,	FALSE},
	{"continue",	CONTINUE,	TRUE,	FALSE},
	{"default",	DEFAULT,	FALSE,	FALSE},
	{"do",		DO,		FALSE,	FALSE},
	{"double",	IDOUBLE,		FALSE,	FALSE},
	{"else",	ELSE,		FALSE,	FALSE},
	{"enum",	ENUM,		FALSE,	FALSE},
	{"extern",	EXTERN,		FALSE,	FALSE},
	{"float",	IFLOAT,		FALSE,	FALSE},
	{"for",		FOR,		TRUE,	FALSE},
	{"goto",	GOTO,		FALSE,	FALSE},
	{"if",		IF,		FALSE,	FALSE},
	{"inline",	INLINE,		FALSE,	TRUE},
	{"int",		IINT,		FALSE,	FALSE},
	{"long",	ILONG,		FALSE,	FALSE},
	{"register",	REGISTER,	FALSE,	FALSE},
	{"restrict",	RESTRICT,	FALSE,	TRUE},
	{"return",	RETURN,		TRUE,	FALSE},
	{"short",	ISHORT,		FALSE,	FALSE},
	{"signed",	SIGNED,		FALSE,	FALSE},
	{"sizeof",	SIZEOF,		FALSE,	FALSE},
	{"static",	STATIC,		FALSE,	FALSE},
	{"struct",	STRUCT,		FALSE,	FALSE},
	{"switch",	SWITCH,		FALSE,	FALSE},
	{"typedef",	TYPEDEF,	FALSE,	FALSE},
	{"union",	UNION,		FALSE,	FALSE},
	{"unsigned",	UNSIGNED,	FALSE,	FALSE},
	{"void",	IVOID,		FALSE,	FALSE},
	{"volatile",	VOLATILE,	FALSE,	FALSE},
	{"while",	WHILE,		FALSE,	FALSE}
};
#define NKEYS (sizeof Keytab / sizeof *Keytab)

lex_env_t *Lex_env;
lexeme_t *Lexeme;

constant_t Constant;
identifier_t Identifier;

char *string_copy(const char *string, int len);

void 
xprint(char *format, ...)
{
	if (DebugLevel)
	{
		va_list aa ;
		va_start(aa, format);
		vfprintf(stdout, format, aa);
		va_end(aa);
	}
	
}
void
lex_error(s)
const char *s;
{
	xprint( "Error: %s", s);
}


const char *
ci_translate_escape(s, p_res)
const char *s;
int *p_res;
{
	static const char hexdigits[] = "0123456789abcdefABCDEF";
	const char *pos, *save_s;
	int ch;

	switch (*s) {
	case 'n':
		ch = '\n';
		break;
	case 't':
		ch = '\t';
		break;
	case 'v':
		ch = '\v';
		break;
	case 'b':
		ch = '\b';
		break;
	case 'r':
		ch = '\r';
		break;
	case 'f':
		ch = '\f';
		break;
	case 'a':
		ch = '\007';
		break;
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
		ch = 0;
		for (save_s = s; isdigit(*s) && *s < '8' && s - save_s < 3; ++s)
			ch = ch * 8 + *s - '0';
		--s;
		break;
	case 'x':
		ch = 0;
		for (; *s != '\0' && (pos = strchr(hexdigits, *s)) != NULL; ++s) {
			if (pos >= hexdigits + 16)
				pos -= 6;
			ch = ch * 16 + pos - hexdigits;
		}
		break;
	default:
		ch = *s;
		break;
	}
	/* Dibyendu : 11/1/99
	 * Fixed problem of sign extension - '\377' is now -1 and not 255
	 */
	*p_res = (int)(char)ch;
	/* *p_res = ch; */

	return s;
}
static void stripcomment(char *line)
{
    char *s = line,  *e = s, instr = 0;
    while (*e)
    {
        if (!instr)
        {
            if (!commentlevel)
            {
                if (*e == '/')
                {
                    if (*(e+1) == '*')
                    {
                        e += 2;
                        *s++ = ' ';
                        commentlevel = 1;
                        continue;
                    }
                    else if (*(e+1) == '/')
                    {
                        *s = 0;
                        return ;
                    }
                }
                else
                    if (*e == '"' ||  *e == '\'')
                        instr =  *e;
            }
            else
            {
                if (*e == '*')
                {
                    if (*(e+1) == '/')
                    {
                        commentlevel = 0;
                        e++;
                    }
                }
                e++;
                continue;
            }
        }
        else
        if (!commentlevel &&  *e == instr)
        {
            int count = 0;
            while (s - count > line && *(s - count - 1) == '\\')
                count++;
            if (!(count &1))
                instr = 0;
        }
        *s++ =  *e++;
    }
    *s = 0;
}

/*  Based on K&P's hoc follow() function.
 */
#define follow(s, ch, ifyes, ifno) ((*(s) == (ch)) ? (++(s), (ifyes)) : (ifno))

static const char *
getline(le)
lex_env_t *le;
{
	static char buf[8192];
	int rv = 0;
	char *q;
	if (le->le_abort_parse)
		return NULL;
	for ( ;; )
	{
		int oldrv = rv;
		while (rv < 4096)
		{
			char *p = (*le->le_getline)(le->le_getline_arg);
			int len;
			if (p == NULL)
				return p;
			++le->le_lnum;
			if (le->le_lines_used)
			{
				if (le->le_lnum >= le->le_lines_used_max*8)
				{
					unsigned char *newdata = (unsigned char *)userAlloc(le->le_lines_used_max + 1000);
					if (newdata)
						
					{
						memcpy(newdata, le->le_lines_used, le->le_lines_used_max);
						userFree(le->le_lines_used);
						le->le_lines_used = newdata;
						le->le_lines_used_max += 1000;
					}
				}
			}
			q = buf + rv;
			strcpy(q, p);
			len = strlen(q);
			rv += len;
			q += len;
			while (len && q > buf && isspace(q[-1]))
				--q, --rv, --len;
			*q = 0;
			if (q > buf && q[-1] != '\\')
				break;
			if (le->le_lines_used)
				le->le_lines_used[le->le_lnum/8] |= (1 << (le->le_lnum & 7));
		}
		stripcomment(buf + oldrv);
		rv = strlen(buf);
		if (buf[0])
			if (commentlevel && rv < 4096)
			{
				buf[rv++] = ' ';
				buf[rv] = 0;
			}
			else
				break;
		else
			rv = 0;
		if (le->le_lines_used)
			le->le_lines_used[le->le_lnum/8] |= (1 << (le->le_lnum & 7));
	}	
	return le->le_line = buf;
}

/*  Skip white space and comments.
 */
static const char *
skip_whitespace(le, line)
lex_env_t *le;
const char *line;
{
	const char aa = 0;
	bool read_another_line;
	bool incomment;

	incomment = FALSE;
	read_another_line = FALSE;

	if (line == NULL)
		line = &aa;
//	if (line == NULL) {
//		if ((line = getline(le)) == NULL)
//			return line;
//	}

	for(;;) {
		while (*line != '\0' && isspace(*line))
			++line;
		if (*line != '\0')
			break;

		if ((line = getline(le)) == NULL)
		{
			if (le->next)
			{
				uninclude(le);
				line = &aa;
				continue;
			}
			break;
		}
		read_another_line = TRUE;
		while (isspace(*line))
			++line;
		if (*line == '#')
		{
			if (le->le_lines_used)
				le->le_lines_used[le->le_lnum/8] |= (1 << (le->le_lnum & 7));
			parse_hash_directive(line + 1, le);
			line += strlen(line);
		}
		else if (preprocessorSkip)
		{
			line += strlen(line);
		}
		else if (le->le_lines_used)
			le->le_lines_used[le->le_lnum/8] |= (1 << (le->le_lnum & 7));
	}

	if (Want_debugging_output && read_another_line) {
#if 0
		putchar('\n');
		xprint("\n\"%s\", %d: %s", le->le_filename, le->le_lnum, line);
#endif
		xprint("\"%s\", %d: %s\n", le->le_filename, le->le_lnum, line);
	}
	if (line && read_another_line)
	{
		replace_macros(line, le);
		xprint(":::%s\n",line);
		while(isspace(*line))
			++line;
		if (DebugLevel)
			xprint("%s(%d):%s\n", le->le_filename, le->le_lnum, line);
	}
	return line;
}

static bool
is_aggr_type_specifier(token)
token_t token;
{
	if (token == STRUCT 
	||  token == UNION
	||  token == ENUM)
		return TRUE;
	return FALSE;
}
    
static bool
is_basic_type_specifier(token)
token_t token;
{
	if (token == IINT
	||  token == UNSIGNED
	||  token == SIGNED
	||  token == ILONG
	||  token == ISHORT
	||  token == IVOID
	||  token == ICHAR
	||  token == IFLOAT
	||  token == IDOUBLE) 
		return TRUE;
	return FALSE;
}

static bool
is_storage_class_or_qualifier(token)
token_t token;
{
	if (token == STATIC
	||  token == EXTERN
	||  token == TYPEDEF
	||  token == AUTO
	||  token == REGISTER
	||  token == ICONST
	||  token == VOLATILE)
		return TRUE;
	return FALSE;
}

static bool
is_decl_specifier(token)
token_t token;
{
	return is_storage_class_or_qualifier(token)
	||     is_basic_type_specifier(token)
	||     is_aggr_type_specifier(token)
	||     token == TYPEDEF_NAME;
}

static token_t Prev_token = 0;	/* remember last token */
static bool Colon_follows = FALSE;

token_t
lex_prev_token(void)
{
	return Prev_token;
}

bool
lex_colon_follows(void)
{
	return Colon_follows;
}

int
lex_search_keyword(const char *line, int len)
{
	char search_buf[33];		
    int top = NKEYS;
    int bottom =  - 1;
    int v;
	if (len > 32)
		return -1;
	memcpy(search_buf, line, len);
	search_buf[len] = '\0';
	
    while (top - bottom > 1)
    {
        int mid = (top + bottom) / 2;
		v = strcmp(search_buf, Keytab[mid].name);
        if (v < 0)
        {
            top = mid;
        }
        else
        {
            bottom = mid;
        }
    }
    if (bottom ==  - 1)
        return -1;
	v = strcmp(search_buf, Keytab[bottom].name);
    if (v)
        return -1;
	if (prm_c99 || !Keytab[bottom].C99)
	    return  bottom;
	return -1;
}

token_t
lex_get_token()
{
	static int pos = -1;
	lex_env_t *le;
	token_t token;
	const char *line;

	le = Lex_env;

	Identifier.id_name[0] = '\0';
	
	if (pos == -1) {
		Want_debugging_output = getenv("LEX_DEBUG") != NULL;
		pos = 0;
	}
	if (le == NULL) {
		if (Want_debugging_output)
			puts("\n");
		return TOK_UNKNOWN;
	}

	if ((line = skip_whitespace(le, le->le_lptr)) == NULL) {
		le->le_lptr = line;
		return TOK_UNKNOWN;	/* EOF */
	}

	switch (*line++) {
	case '_': case '$':

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	case 'v': case 'w': case 'x': case 'y': case 'z': 

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	case 'V': case 'W': case 'X': case 'Y': case 'Z':
		{
			const char *s;
			int len, i;

			--line;
			if (*line == 'L')
			{
				if (line[1] == '"')
				{
					line += 2;
					token = get_string(le, line, &Constant);
					Lexeme->constant = &Constant;
					line = le->le_lptr;
				}
				else if (line[1] == '\'')
				{
					line += 2;
					goto grabchar;
				}
			}
			for (s = line; isalnum(*s) || *s == '_' || *s == '$'; ++s)
				;
			len = s - line;

			i = lex_search_keyword(line, len);
			if (i >= 0)
			{
				token = Keytab[i].token;
				line += len;
				break;
			}
					
			if (len+1 > sizeof Identifier.id_name)
				len = sizeof Identifier.id_name-1;
			
			strncpy(Identifier.id_name, line, len+1);
			Identifier.id_name[len] = '\0';
			Lexeme->identifier = &Identifier;

			line = skip_whitespace(le, s);
            if (line == NULL)
            {
        		le->le_lptr = line;
		        return 0;	/* EOF */
            }
			/* The parser provides the function name_type() which is 
			 * called here to determine whether a name is a potential 
			 * TYPEDEF name. 
			 */
			token = name_type(Identifier.id_name);
			Colon_follows = *line == ':';
		}
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		{
			char *end;
			long val;
			
			val = strtol(line - 1, &end, 0);
			if (end == line - 1) {
				le->le_lptr = line;
				xprint(
					"Badly formed integer constant \"%s\"",
					line - 1);
				token = BADTOK;
			}
			else if (*end == 'e' || *end == 'E' || *end == '.') {
				token = get_float_constant(le, line-1, &line,
								&Constant);
				Lexeme->constant = &Constant;
			}
			else {
				while (*end == 'L' || *end == 'l' || *end == 'u' || *end == 'U')
					++end;
				Constant.co_val = string_copy(line-1, end-(line-1));
				Constant.co_size = end-(line-1);
				line = end;

				Lexeme->constant = &Constant;
				token = INTEGER_CONSTANT;
			}
		}
		break;
	case '!':
		token = follow(line, '=', NOTEQ, NOT);
		break;
	case '=':
		token = follow(line, '=', EQEQ, EQUALS);
		break;
	case '%':
		token = follow(line, '=', PERCENT_EQUALS, PERCENT);
		break;
	case '/':
		token = follow(line, '=', SLASH_EQUALS, SLASH);
		break;
	case '^':
		token = follow(line, '=', XOR_EQUALS, XOR);
		break;
	case '*':
		token = follow(line, '=', STAR_EQUALS, STAR);
		break;
	case '[':
		token = LBRAC;
		break;
	case ']':
		token = RBRAC;
		break;
	case '{':
		token = LBRACE;
		break;
	case '}':
		token = RBRACE;
		break;
	case '(':
		token = LPAREN;
		break;
	case ')':
		token = RPAREN;
		break;
	case ',':
		token = COMMA;
		break;
	case ';':
		token = SEMI;
		break;
	case '?':
		token = QUERY;
		break;
	case ':':
		token = COLON;
		break;
	case '\'': 
grabchar:
		{
		/*  BUG: no escapes etc.
		 */
		int val;
		const char *startp = line-1;
		const char *endp = 0;

		if (*line == '\\')
			line = ci_translate_escape(line + 1, &val);
		else
			val = *line;
		++line;

		if (*line != '\'') {
			le->le_lptr = line;
			xprint( "Unterminated char constant");
			token = BADTOK;
		}
		else {
			endp = ++line;
			Constant.co_val = string_copy(startp, endp-startp);
			Constant.co_size = endp-startp;
			Lexeme->constant = &Constant;
			token = CHARACTER_CONSTANT;
		}
		break;
	}
	
	case '"': {

		token = get_string(le, line, &Constant);
		Lexeme->constant = &Constant;
		line = le->le_lptr;
		break;
	}
	case '.':
		if (*line == '.' && line[1] == '.') {
			line += 2;
			token = ELLIPSIS;
		}
		else if (isdigit(*line)) {
			token = get_float_constant(le, line-1, &line, &Constant);
			Lexeme->constant = &Constant;
		}
		else
			token = DOT;
		break;
	case '~':
		token = TILDE;
		break;
	case '+':
		if (*line == '+')
			token = PLUSPLUS;
		else if (*line == '=')
			token = PLUS_EQUALS;
		else {
			token = PLUS;
			--line;
		}
		++line;
		break;
	case '-':
		if (*line == '>')
			token = ARROW;
		else if (*line == '-')
			token = MINUSMINUS;
		else if (*line == '=')
			token = MINUS_EQUALS;
		else {
			token = MINUS;
			--line;
		}
		++line;
		break;
	case '|':
		if (*line == '|')
			token = OROR;
		else if (*line == '=')
			token = OR_EQUALS;
		else {
			--line;
			token = OR;
		}
		++line;
		break;
	case '&':
		if (*line == '&')
			token = ANDAND;
		else if (*line == '=')
			token = AND_EQUALS;
		else {
			--line;
			token = AND;
		}
		++line;
		break;
	case '>':
		if (*line == '>') {
			++line;
			token = follow(line, '=', RSHIFT_EQUALS, RSHIFT);
		}
		else if (*line == '=') {
			++line;
			token = GTEQ;
		}
		else
			token = GREATERTHAN;
		break;
	case '<':
		if (*line == '<') {
			++line;
			token = follow(line, '=', LSHIFT_EQUALS, LSHIFT);
		}
		else if (*line == '=') {
			++line;
			token = LESSEQ;
		}
		else
			token = LESSTHAN;
		break;
			
	default:
		le->le_lptr = line; /* because we are about to call diagf */
		xprint(
			"Illegal character '%c' (0x%02x)", line[-1], line[-1]);
		xprint( "%s:%d\n", le->le_filename, le->le_lnum);
		token = BADTOK;
		break;
	}
	le->le_lptr = line;

#if 0
	if (Want_debugging_output) {
		const char *name;

		if (pos > 70) {
			putchar('\n');
			pos = 0;
		}
		name = tokname(token);
		xprint("%s ", name);
		pos += strlen(name) + 1;
		fflush(stdout);
	}
#endif

	Prev_token = token;
	return token;
}

static int
get_string(le, line, co)
lex_env_t *le;
const char *line;
constant_t *co;
{
	static const char badalloc[] =
				"Unable to allocate memory for string constant";
	char *buf;
	int bufsize = 0;
	int opos;
	bool ok;

	if (bufsize == 0) {
		bufsize = 80;
		if ((buf = malloc(bufsize + 1)) == NULL) {
			xprint( "%s", badalloc);
			return BADTOK;
		}
	}

	opos = 0;
	ok = FALSE;		/* set to TRUE on success */

	for (; *line != '\0'; ++line) {
		int ch;

		if (*line == '"') {
			const char *new_line;

			new_line = skip_whitespace(le, line + 1);
			if (new_line == NULL || *new_line != '"') {
				ok = TRUE;
				le->le_lptr = new_line;
				break;
			}

			line = new_line;
			continue;
		}

		if (*line != '\\')
			ch = *line;
		else if (*++line == '\n') {
			line = getline(le);
			ch = (line != NULL) ? *line : '\0';
		}
		else
			line = ci_translate_escape(line, &ch);

		if (line == NULL || *line == '\n' || *line == '\0') {
			le->le_lptr = line;
			xprint(
						"Unterminated string constant");
			break;
		}

		if (opos == bufsize) {
			bufsize *= 2;
			if ((buf = realloc(buf, bufsize + 1)) == NULL) {
				le->le_lptr = line;
				xprint(
							"%s", badalloc);
				break;
			}
		}
		buf[opos++] = ch;
	}
	buf[opos++] = '\0';

	if (!ok)
	{
		le->le_lptr = line;
		free(buf);
		return BADTOK;
	}

	co->co_val = string_copy(buf, strlen(buf));
	free(buf);
	co->co_size = opos;
	return STRING_CONSTANT;
}

static int
get_float_constant(le, line, p_end, co)
lex_env_t *le;
const char *line, **p_end;
constant_t *co;
{
	double val;
	char *end;

	val = strtod(line, &end);

	if (end == line) {
		le->le_lptr = line;
		xprint( "Badly formed floating constant \"%s\"", line);
		return BADTOK;
	}

	co->co_val = string_copy(line, end-line);
	co->co_size = end-line;

	*p_end = end;

	return FLOATING_CONSTANT;
}

/* static */
const char *
tokname(token_t token)
{
	static struct {
		const char *name;
		token_t token;
	} tab[] = {
		"BOOL",                   IBOOLEAN,
		"COMPLEX",                COMPLEX,
		"IMAGINARY",              IMAGINARY,
		"VOID",                   IVOID,
		"CHAR",                   ICHAR,
		"SHORT",                  ISHORT,
		"INT",                    IINT,
		"LONG",                   ILONG,
		"FLOAT",                  IFLOAT,
		"IDOUBLE",                 IDOUBLE,
		"SIGNED",                 SIGNED,
		"UNSIGNED",               UNSIGNED,
		"CONST",                  ICONST,
		"VOLATILE",               VOLATILE,
		"RESTRICT",               RESTRICT,
		"IF",                     IF,
		"ELSE",                   ELSE,
		"WHILE",                  WHILE,
		"FOR",                    FOR,
		"DO",                     DO,
		"GOTO",                   GOTO,
		"BREAK",                  BREAK,
		"CONTINUE",               CONTINUE,
		"RETURN",                 RETURN,
		"SWITCH",                 SWITCH,
		"CASE",                   CASE,
		"DEFAULT",                DEFAULT,
		"SIZEOF",                 SIZEOF,
		"AUTO",                   AUTO,
		"REGISTER",               REGISTER,
		"STATIC",                 STATIC,
		"EXTERN",                 EXTERN,
		"TYPEDEF",                TYPEDEF,
		"INLINE",                 INLINE,
		"STRUCT",                 STRUCT,
		"UNION",                  UNION,
		"ENUM",                   ENUM,
		"AND",                    AND,
		"TILDE",                  TILDE,
		"NOT",                    NOT,
		"LESSTHAN",               LESSTHAN,
		"GREATERTHAN",            GREATERTHAN,
		"XOR",                    XOR,
		"OR",                     OR,
		"PLUS",                   PLUS,
		"MINUS",                  MINUS,
		"SLASH",                  SLASH,
		"PERCENT",                PERCENT,
		"STAR",                   STAR,
		"DOT",                    DOT,
		"COLON",                  COLON,
		"QUERY",                  QUERY,
		"SEMI",                   SEMI,
		"COMMA",                  COMMA,
		"LPAREN",                 LPAREN,
		"RPAREN",                 RPAREN,
		"LBRACE",                 LBRACE,
		"RBRACE",                 RBRACE,
		"LBRAC",                  LBRAC,
		"RBRAC",                  RBRAC,
		"EQUALS",                 EQUALS,
		"STAR_EQUALS",            STAR_EQUALS,
		"SLASH_EQUALS",           SLASH_EQUALS,
		"PERCENT_EQUALS",         PERCENT_EQUALS,
		"PLUS_EQUALS",            PLUS_EQUALS,
		"MINUS_EQUALS",           MINUS_EQUALS,
		"LSHIFT_EQUALS",          LSHIFT_EQUALS,
		"RSHIFT_EQUALS",          RSHIFT_EQUALS,
		"AND_EQUALS",             AND_EQUALS,
		"XOR_EQUALS",             XOR_EQUALS,
		"OR_EQUALS",              OR_EQUALS,
		"ANDAND",                 ANDAND,
		"OROR",                   OROR,
		"EQEQ",                   EQEQ,
		"NOTEQ",                  NOTEQ,
		"GTEQ",                   GTEQ,
		"LESSEQ",                 LESSEQ,
		"LSHIFT",                 LSHIFT,
		"RSHIFT",                 RSHIFT,
		"PLUSPLUS",               PLUSPLUS,
		"MINUSMINUS",             MINUSMINUS,
		"ARROW",                  ARROW,
		"ELLIPSIS",               ELLIPSIS,
		"STRING_CONSTANT",        STRING_CONSTANT,
		"INTEGER_CONSTANT",       INTEGER_CONSTANT,
		"CHARACTER_CONSTANT",     CHARACTER_CONSTANT,
		"FLOATING_CONSTANT",      FLOATING_CONSTANT,
		"IDENTIFIER",             IDENTIFIER,
		"TYPEDEF_NAME",           TYPEDEF_NAME,
		"BADTOK",                 BADTOK,
		"EOF",                    0,
	};
	static char buf[100];
	int i;

	for (i = 0; i < sizeof tab / sizeof *tab; ++i)
		if (tab[i].token == token)
			return tab[i].name;

	(void) sprintf(buf, "<unknown token %d>", token);
	return buf;
}

char *string_copy(const char *string, int len)
{
	char *p;
	p = allocate(len+1);
	strncpy(p, string, len);
	p[len] = 0;
	return p;
}
