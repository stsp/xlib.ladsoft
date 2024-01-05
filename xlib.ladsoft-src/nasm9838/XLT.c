#include <stdio.h>


struct entry {
    char *str, *color ;
} ;
static char *builtins[] = {
	"global","extern","section","virtual","dword","byte","class",
	"use16","use32","export","import","word","near","short","far",
    "fword","qword","tbyte","group",0
} ;
static char *conditions[] = {
    "a", "ae", "b", "be", "c", "e", "g", "ge", "l", "le",
    "na", "nae", "nb", "nbe", "nc", "ne", "ng", "nge", "nl", "nle", "no",
    "np", "ns", "nz", "o", "p", "pe", "po", "s", "z",0
} ;
static char *directives[] = {
    "%arg",
    "%assign", "%clear", "%define", "%elif", "%elifctx", "%elifdef",
    "%elifid", "%elifidn", "%elifidni", "%elifmacro", "%elifnctx", "%elifndef",
    "%elifnid", "%elifnidn", "%elifnidni", "%elifnmacro", "%elifnnum", "%elifnstr",
    "%elifnum", "%elifstr", "%else", "%endif", "%endm", "%endmacro",
    "%endrep", "%error", "%exitrep", "%iassign", "%idefine", "%if",
    "%ifctx", "%ifdef", "%ifid", "%ifidn", "%ifidni", "%ifmacro", "%ifnctx",
    "%ifndef", "%ifnid", "%ifnidn", "%ifnidni", "%ifnmacro", "%ifnnum",
    "%ifnstr", "%ifnum", "%ifstr", "%imacro", "%include",
    "%ixdefine", "%line",
    "%local",
    "%macro", "%pop", "%push", "%rep", "%repl", "%rotate",
    "%stacksize",
    "%strlen", "%substr", "%undef", "%xdefine",0
};

struct entry strs[1000] ;
int count = 0 ;
void insert1(char *str, char *clr)
{
    int i;
    printf("%s\n",str) ;
    for (i=0; i < strlen(str); i++)
        str[i] = tolower(str[i]) ;
    for (i=0; i < count; i++)
        if (!strcmp(strs[i].str,str))
            return ;
    strs[count].str = strdup(str) ;
    strs[count].color = clr ;
    count++ ;
}
void insert(char *str, char *clr)
{
    int i ;
    char *p = strchr(str,'*'),**q ;
    if (!strcmp(str + strlen(str)-2,"cc")) {
        if (str[0] == 'J') {
            insert("JECXZ","keywordColor") ;
            insert("JCXZ","keywordColor") ;
        }
        p = str + strlen(str)-2 ;
        q = conditions ;
        while (*q) {
            strcpy(p,*q) ;
            insert1(str,clr) ;
            q++ ;
        }
    } else if (!p) {
        insert1(str,clr) ;
    } else
        for (i=0; i < 8; i++) {
            *p = '0' + i ;
            insert1(str,clr) ;

        }
        
}
void xsort(void)
{
    int i,j ;
    for (i=0; i < count-1; i++) 
        for (j=i+1; j < count; j++)
            if (strcmp(strs[j].str,strs[i].str) < 0) {
                struct entry temp = strs[j] ;
                strs[j] = strs[i] ;
                strs[i] = temp ;
            }
}
main()
{
    int i ;
    FILE *in = fopen("insns.dat","r") ;
    FILE *out = fopen("asm_kw.dat","w") ;
    char **p = directives ;
    while (*p) {
        insert(*p,"escapeColor") ;
        p++ ;
    }
	p = builtins ;
	while (*p) {
		insert(*p,"keywordColor") ;
		p++ ;
	}
    while (!feof(in)) {
        char buf[256],*p,*q ;
        buf[0] = 0 ;
        fgets(buf,256,in) ;
        if (isalpha(buf[0])) {
            p = strchr(buf,' ') ;
            q = strchr(buf,'\t') ;
            if (p)
                *p = 0;
            if (q)
                *q = 0 ;
            insert(buf,"keywordColor") ;
        }
    }
    fclose(in) ;

    in = fopen("regs.dat","r") ;
    while (!feof(in)) {
        char buf[256],*p ;
        buf[0] = 0 ;
        fgets(buf,256,in) ;
        if (isalpha(buf[0])) {
            p = strchr(buf,' ') ;
            if (!p)
                p = strchr(buf,'\t') ;
            *p = 0;
            insert(buf,"numberColor") ;
        }
    }
    xsort() ;
    for (i=0; i < count; i++)
        fprintf(out,"{ \"%s\",&%s },\n",strs[i].str, strs[i].color) ;
}