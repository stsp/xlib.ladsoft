typedef struct
{
    unsigned char *string;
    char argcount;
    char **args;
    int flags ;
#define DS_VARARGS 1
#define DS_PERMANENT 2
} defines_t;

void preprocini(void);
lex_env_t *parse_hash_directive(const char *line, lex_env_t *le);
lex_env_t * doerror(const char *line, lex_env_t *le);             /* STATIC */
lex_env_t * dopragma(const char *line, lex_env_t *le);            /* STATIC */
lex_env_t *Compile_Pragma(const char *line, lex_env_t *le);
lex_env_t * doline(const char *line, lex_env_t *le);              /* STATIC */
FILE *srchpth(char *name, char *path, char * attrib, int sys);
lex_env_t * doinclude(const char *line, lex_env_t *le);           /* STATIC */
lex_env_t *uninclude(lex_env_t *current);
void glbdefine(char *name, char *value);
lex_env_t * dodefine(const char *line, lex_env_t *le);
lex_env_t * doundef(const char *line, lex_env_t *le);
void nodefines(void);
int indefine(symbol_t *sp);
void enterdefine(symbol_t *sp);
void exitdefine(void);
int defid(char *name, char **p, char *q);
int definsert(char *macro, char *end, char *begin, char *text, int len, int replen);
int defstringize(char *macro, char *end, char *begin, char *text, int len, int replen);
int defreplaceargs(char *macro, int count, char **oldargs, char **newargs, char *varargs);
void deftokenizing(char *macro);
char *fullqualify(const char *string);
void filemac(char *string, lex_env_t *le);
void datemac(char *string, lex_env_t *le);
void timemac(char *string, lex_env_t *le);
void linemac(char *string, lex_env_t *le);
void defmacroreplace(char *macro, char *name, lex_env_t *le);
int replacesegment(char *start, char *end, int *inbuffer, int totallen, char **pptr, lex_env_t *le);
int replace_macros(char *line, lex_env_t *le);
void repdefines(char *lptr);                                      /* STATIC */
void pushif(lex_env_t *le);
void popif(lex_env_t *le);
lex_env_t * doifdef(int flag, const char *line, lex_env_t *le);
lex_env_t * doif(int flag, const char *line, lex_env_t *le);
lex_env_t * doelif(const char *line, lex_env_t *le);
lex_env_t * doelse(const char *line, lex_env_t *le);
lex_env_t * doendif(const char *line, lex_env_t *le);

extern bool preprocessorSkip;
extern char *prm_searchpath ; 
extern char *sys_searchpath ;
