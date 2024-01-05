/* 
XRC Resource Compiler
Copyright 1997, 1998 Free Software Foundation, Inc.
Written by Ian Lance Taylor, Cygnus Support.
Copyright 2006-2011 David Lindauer.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

You may contact the author of this derivative at:
	mailto::camille@bluegrass.net
 */
/* 
 * error handler
 */
#include <stdio.h>
#include <string.h>
#include "preproc.h"
#include "lists.h"

extern int prm_errfile;
extern FILE *errFile;
extern FILE *listFile;
extern int errlineno;
extern char *errfile;
extern int prm_asmfile;
extern FILE *outputFile;
extern int prm_maxerr;
extern int prm_diag;
extern int prm_listfile;
extern int lastch;
extern enum e_sym lastst;
extern char lastid[];
extern int lineno;
extern FILE *inclfile[10]; /* shared with preproc */
extern int incldepth; /* shared with preproc */
extern char *infile;
extern int prm_warning, prm_cplusplus, prm_extwarning, prm_quieterrors;

int diagcount = 0;
int referrorlvl = 3;
ERRORS *errlist = 0;
static ERRORS *errtail = 0;
static ERRORS *curerr = 0;
static char expectlist[] = 
{
    "###################################################:{}.#])#,;"
};
static int errline;

int total_errors = 0;
void initerr(void)
{
    errlist = errtail = curerr = 0;
    total_errors = 0;
    diagcount = 0;
    errline = 0;
}

//-------------------------------------------------------------------------

#ifdef DIAGNOSTICS
    void diag(char *s)
    /*
     * internal diags come here
     */
    {
        diagcount++;
        if (prm_diag)
        {
            printf("DIAG - %s\n", s);
            if (prm_errfile && errFile)
                fprintf(errFile, "/*DIAG - %s*/", s);
            if (prm_listfile && listFile)
                fprintf(listFile, "/*DIAG - %s*/", s);
            if (prm_asmfile)
                fprintf(outputFile, "DIAG - %s\n", s);
        }
    }
#endif 
int printerr(char *buf, ERRORS *err)
/*
 * subroutine gets the error code and returns whether it is an error or
 * warning
 */
{
    int errlvl = 0;
    switch (err->errornumber)
    {
        case ERR_PUNCT:
            sprintf(buf, "Expected '%c'", expectlist[(int)err->data]);
            break;
        case ERR_INSERT:
            sprintf(buf, "Inserted '%c'", expectlist[(int)err->data]);
            break;
        case ERR_NEEDCHAR:
            sprintf(buf, "Expected '%c'", (char)err->data);
            break;
        case ERR_ILLCHAR:
            sprintf(buf, "Illegal character '%c'", (char)err->data);
            break;
        case ERR_NEEDCONST:
            sprintf(buf, "Constant value expected");
            break;
        case ERR_UNDEFINED:
            sprintf(buf, "Undefined symbol '%s'", (char*)err->data);
            break;
        case ERR_DUPSYM:
            sprintf(buf, "Duplicate symbol '%s'", (char*)err->data);
            break;
        case ERR_IDENTEXPECT:
            sprintf(buf, "Expected '%s'", (char*)err->data);
            break;
        case ERR_IDEXPECT:
            sprintf(buf, "Identifier expected");
            break;
        case ERR_PREPROCID:
            sprintf(buf, "Invalid preprocessor directive '%s'", (char*)err
                ->data);
            break;
        case ERR_INCLFILE:
            sprintf(buf, "File name expected in #include directive");
            break;
        case ERR_CANTOPEN:
            sprintf(buf, "Cannot open file \"%s\" for read access", (char*)err
                ->data);
            break;
        case ERR_EXPREXPECT:
            sprintf(buf, "Expression expected");
            break;
        case ERR_UNEXPECT:
            if (lastst == ident)
                sprintf(buf, "Unexpected '%s'", lastid);
            else
                sprintf(buf, "Unexpected '%c'", lastch);
            break;
        case ERR_PREPROCMATCH:
            sprintf(buf, "Unbalanced preprocessor directives");
            break;
        case ERR_MACROSUBS:
		case ERR_WRONGMACROARGS:
            sprintf(buf, "Macro substitution error");
            break;
        case ERR_ERROR:
            sprintf(buf, "User error: %s", (char*)err->data);
            break;
        case ERR_INTERP:
            sprintf(buf, "%s", (char*)err->data);
            break;
        case ERR_COMMENTMATCH:
            sprintf(buf, "File ended with comment in progress");
            break;
        case ERR_STRINGTOOBIG:
            sprintf(buf, "String constant too long");
            break;
        case ERR_CONSTTOOLARGE:
            sprintf(buf, "Numeric constant is too large");
            break;
        case ERR_INVALIDSTRING:
            sprintf(buf, "Invalid string operation");
            break;
        case ERR_PREPIG:
            sprintf(buf, "Preprocessor directive ignored");
            errlvl = 1;
            break;
        case ERR_CHAR4CHAR:
            sprintf(buf, "Character constant must be 1 to 4 characters");
            break;
        case ERR_USERERR:
            sprintf(buf, "User: %s", (char*)err->data);
            break;
        case ERR_USERWARN:
            sprintf(buf, "User: %s", (char*)err->data);
            errlvl = 1;
            break;
        case ERR_EXTRA_DATA_ON_LINE:
            sprintf(buf, "Extra data on line");
            break;
        case ERR_BEGIN_EXPECTED:
            sprintf(buf, "BEGIN expected");
            break;
        case ERR_END_EXPECTED:
            sprintf(buf, "END expected");
            break;
        case ERR_RESOURCE_ID_EXPECTED:
            sprintf(buf, "resource identifier expected");
            break;
        case ERR_STRING_EXPECTED:
            sprintf(buf, "string expected");
            break;
        case ERR_ACCELERATOR_CONSTANT_EXPECTED:
            sprintf(buf, "Accelerator key expected");
            break;
        case ERR_NO_ASCII_VIRTKEY:
            sprintf(buf, 
                "ASCII/VIRTKEY keywords not allowed for string key type");
                break;
        case ERR_NOT_DIALOGEX:
            sprintf(buf, "Need DIALOGEX for this feature");
            break;
        case ERR_UNKNOWN_DIALOG_CONTROL_CLASS:
            sprintf(buf, "Unknown control type");
            break;
        case ERR_VERSIONINFO_TYPE_1:
            sprintf(buf, "VERSIONINFO id must be 1");
            break;
        case ERR_UNKNOWN_RESOURCE_TYPE:
            sprintf(buf, "Unknown Resource Type");
            break;
        case ERR_INVALIDCLASS:
            sprintf(buf, "class or ID expected");
            break;
        case ERR_FIXEDDATAEXPECTED:
            sprintf(buf, "Fixed version info expected");
            break;
        case ERR_BLOCK_EXPECTED:
            sprintf(buf, "BLOCK keyword expected");
            break;
        case ERR_INVALID_VERSION_INFO_TYPE:
            sprintf(buf, "Invalid version type block");
            break;
        default:
            sprintf(buf, "Error #%d", err->errornumber);
            break;
    }
    return errlvl;
}

//-------------------------------------------------------------------------

void lferror(void)
/*
 * sticck an error in the list file
 */
{
    char buf[100];
    while (curerr)
    {
        int errlvl = printerr(buf, curerr);
        if (!(errlvl &1))
        {
            if (prm_listfile)
                fprintf(listFile, "**** ERROR: %s\n", buf);
        }
        else if (prm_warning && (prm_extwarning || !(errlvl &2)))
        {
            if (prm_listfile)
                fprintf(listFile, "** WARNING: %s\n", buf);
        }
        curerr = curerr->link;
    }

}

//-------------------------------------------------------------------------

void basicskim(int *skimlist)
/*
 * simple skim for a token with no nesting
 */
{
    int i;
    for (i = 0;; i++)
    {
        if (lastst == skimlist[i] || lastst == eof)
            break;
        if (skimlist[i] == 0)
        {
            getsym();
            i = 0;
        }
    }
}

/*
 * the following routines do token skimming and keep track of parenthesis
 * and brace nesting levels as well
 */
BALANCE *newbalance(BALANCE *bal)
{
    BALANCE *rv = AllocateMemory(sizeof(BALANCE));
    rv->back = bal;
    rv->count = 0;
    if (lastst == openpa)
        rv->type = BAL_PAREN;
    else
        rv->type = BAL_BRACKET;
    return (rv);
}

//-------------------------------------------------------------------------

void setbalance(BALANCE **bal)
{
    if (*bal == 0)
        if (lastst = openpa || lastst == closepa)
            *bal = newbalance(*bal);
        else
            return ;
    switch (lastst)
    {
        case closepa:
            while (*bal && (*bal)->type != BAL_PAREN)
            {
                (*bal) = (*bal)->back;
            }
            if (!((*bal)->type)--)
                (*bal) = (*bal)->back;
            else
                return ;
        case closebr:
            while (*bal && (*bal)->type != BAL_BRACKET)
            {
                (*bal) = (*bal)->back;
            }
            if (!((*bal)->type)--)
                (*bal) = (*bal)->back;
        case openpa:
            if ((*bal)->type != BAL_PAREN)
                *bal = newbalance(*bal);
            (*bal)->count++;
            break;

        case openbr:
            if ((*bal)->type != BAL_BRACKET)
                *bal = newbalance(*bal);
            (*bal)->count++;
            break;
    }
    return ;
}

//-------------------------------------------------------------------------

void expskim(int *skimlist)
{
    BALANCE *bal = 0;
    int i = 0;
    for (i = 0;; i++)
    {
        if (lastst == openpa || lastst == openbr)
        {
            setbalance(&bal);
            getsym();
        }
        else
            if (lastst == eof)
                break;
            else
                if (lastst == skimlist[i])
        if (lastst == closepa || lastst == openpa)
        {
            if (!bal)
                        break;
            setbalance(&bal);
            getsym();
        }
        else
            break;
        else
        if (skimlist[i] == 0)
        {
            i = 0;
            getsym();
        }
    }

}

//-------------------------------------------------------------------------

void basicerror(int n, void *data)
/*
 * standard routine for putting out an error
 */
{
    char buf[100];
    ERRORS *nexterr;
    int errlvl, errored = 0;
    ;
    nexterr = AllocateMemory(sizeof(ERRORS));
    nexterr->errornumber = n;
    nexterr->link = 0;
    nexterr->data = data;
    if (errlist == 0)
        errlist = errtail = nexterr;
    else
    {
        errtail->link = nexterr;
        errtail = nexterr;
    }
    errlvl = printerr(buf, nexterr);
    if (curerr == 0)
        curerr = nexterr;
    if (!(errlvl &1))
    {
        errline = lineno;
        if (!prm_quieterrors)
            fprintf(stdout, "Error   %s(%d):  %s", errfile, errlineno, buf);
        if (prm_errfile)
            fprintf(errFile, "Error   %s(%d):  %s", errfile, errlineno, buf);
        errored++;
        total_errors++;
    }
    else if (prm_warning && (errline != lineno) && (prm_extwarning || !(errlvl
        &2)))
    {
        errored++;
        if (!prm_quieterrors)
            fprintf(stdout, "Warning %s(%d):  %s", errfile, errlineno, buf);
        if (prm_errfile)
            fprintf(errFile, "Warning %s(%d):  %s", errfile, errlineno, buf);
    }
    if (errored)
    {
        if (!prm_quieterrors)
            fputc('\n', stdout);
        if (prm_errfile)
            fputc('\n', errFile);
    }
    if (total_errors > prm_maxerr)
    {
        exit(1);
    }
}

//-------------------------------------------------------------------------

void Error(char *string)
/*
 * some of the library functions required a generic error function
 *
 * we are remapping it to the C/C++ error routines
 */
{
    basicerror(ERR_INTERP, (void*)string);
}

//-------------------------------------------------------------------------

void generror(int n, int data, int *skimlist)
/*
 * most errors come here
 */
{

    basicerror(n, (void*)data);
    if (skimlist)
        basicskim(skimlist);
}

//-------------------------------------------------------------------------

void gensymerror(int n, char *data)
/*
 * errors come here if the error has a symbol name
 */
{
    char buf[100];
    if (data)
        strcpy(buf, data);
    else
        buf[0] = 0;
    basicerror(n, (void*)litlate(buf));
}

/*
 * various utilities for special case errors
 */
void expecttoken(int n, int *skimlist)
{
    if (skimlist)
        generror(ERR_PUNCT, n, skimlist);
    else
        generror(ERR_INSERT, n, 0);
}

//-------------------------------------------------------------------------

void generrorexp(int n, int data, int *skimlist)
{
    basicerror(n, (void*)data);
    if (skimlist)
        expskim(skimlist);
}

//-------------------------------------------------------------------------

void gensymerrorexp(int n, char *data)
{
    basicerror(n, (void*)litlate(data));
}

//-------------------------------------------------------------------------

void expecttokenexp(int n, int *skimlist)
{
    if (skimlist)
        generrorexp(ERR_PUNCT, n, skimlist);
    else
        generrorexp(ERR_INSERT, n, 0);
}
