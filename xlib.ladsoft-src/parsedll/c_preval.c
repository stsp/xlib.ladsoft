/* 
   Copyright 1994-2011 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  

   You may contact the author at:

   mailto::camille@bluegrass.net

*/
/*
 * Evaluate an expression which should be known at compile time.
 * This uses recursive descent.  It is roughly analogous to the
 * primary expression handler except it returns a value rather than
 * an enode list
 */
#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <ctype.h>
 
#include "c_lex.h"
#include "c_list.h"
#include "c_parser.h"
#include "c_preprc.h" 
#include "c_preval.h"
 
static int ieprimary(const char **line)   
/*
 * PRimary integer
 *    id
 *    iconst
 *    (cast )intexpr
 *    (intexpr)
 */
{
			int val = 0;
        symbol_t     *sp;
		int needclose ;
		while (isspace(**line))
			++(*line);
        if(isalpha((*line)[0]) || (*line)[0] == '_') {
		   // the #defines have already been replaced, must be an unknown symbol
		   // which evaluates to 0
		   	char id[MAX_IDENTIFIER_LEN],*p = id;
			while (isalpha(**line) || (**line) == '_')
				*p++ = *(*line)++;
			*p = 0;
#ifdef XXXXX
			sp = find_symbol(&defines, id, TRUE);
			if (sp)
			{
				defines_t *defs = sp->data;
				val = !!(atoi(defs->string));
			}
#endif
        }
		else if (isdigit(**line))
		{
			val = strtol(*line, line, 10);
		}
		else if (**line == '(')
		{
			++(*line);
			val = preproc_eval(line);
			while(isspace(**line))
				++(*line);
			if (**line == ')')
				++(*line);
			else
				xprint("preproc_eval: need )\n");
		}
         else {
			 xprint( "preproc_eval: unknown symbol\n");
          }
        return val;
}
/*
 * Integer unary
 *   - unary
 *   ! unary
 *   ~unary
 *   primary
 */
static int ieunary(const char **line)
{
   int temp;
	while (isspace(**line))
		++(*line);
	switch (**line) {
		case '-':
				++(*line);
				temp = -ieunary(line);
				break;
		case '!':
				++(*line);
				temp = !ieunary(line);
				break;
		case '~':
				++(*line);
				temp = ~ieunary(line);
				break;
      case '+':
            ++(*line) ;
            temp = ieunary(line) ;
            break ;
		default:
				temp = ieprimary(line);
				break;
	}
	while (isspace(**line))
		++(*line);
	return(temp);
}
static int iemultops(const char **line)
/* Multiply ops */
{
   int val1 = ieunary(line),val2;
	while (**line == '*' || **line == '/' || **line == '%') {
		char oper = *(*line)++;
		val2 = ieunary(line);
		switch(oper) {
			case '*':
					val1 = val1 * val2;
					break;
			case '/':
					val1 = val1 / val2;
					break;
			case '%':
					val1 = val1 % val2;
					break;
		}
	}
	return(val1);
}
static int ieaddops(const char **line)
/* Add ops */
{
   int val1 = iemultops(line),val2;
	while (**line == '+' || **line == '-')	{
		char oper = *(*line)++;
		val2 = iemultops(line);
		if (oper == '+') 
			val1 = val1 + val2;
		else
			val1 = val1 - val2;
	}
	return(val1);
}
static int ieshiftops(const char **line)
/* Shift ops */
{
   int val1 = ieaddops(line), val2;
	while (**line == (*line)[1] && (**line == '<' || **line == '>')) {
		char oper = *(*line)++;
		
		++(*line);
		val2 = ieaddops(line);
		if (oper == '<')
			val1 <<= val2;
		else
			val1 >>= val2;
	}
	return(val1);
}
static int ierelation(const char **line)
/* non-eq relations */
{
   int val1 = ieshiftops(line), val2;
	while ((**line == '<' || **line == '>') && (*line)[1] != **line)
	{
		char oper = **line;
		bool eq = FALSE;
		++(*line);
		if (**line == '=')
		{
			eq = TRUE;
			++(*line);
		}
		val2 = ieshiftops(line);
		switch(oper) {
			case '<':
					if (eq)
						val1 = val1 <= val2;
					else
						val1 = val1 < val2;
					break;
			case '>':
					if (eq)
						val1 = val1 >= val2;
					else
						val1 = val1 > val2;
					break;
		}
	}
	return(val1);
}
static int ieequalops(const char **line)
/* eq relations */
{
   int val1 = ierelation(line),val2;
	while ((*line)[1] == '=' && (**line == '=' || **line == '!')) {
		char oper = **line;
		*line += 2;
		val2 = ierelation(line);
		if (oper == '!')
			val1 = val1 != val2;
		else
			val1 = val1 == val2;
	}
	return(val1);
}
static int ieandop(const char **line)
/* and op */
{
   int val1 = ieequalops(line),val2;
	while (**line == '&' && (*line)[1] != '&') {
		++(*line);
		val2 = ieequalops(line);
		val1 = val1 & val2;
	}
	return(val1);
}
static int iexorop(const char **line)
/* xor op */
{
   int val1 = ieandop(line),val2;
	while (**line == '^') {
		++(*line);
		val2 = ieandop(line);
		val1 = val1 ^ val2;
	}         
	return(val1);
}
static int ieorop(const char **line)
/* or op */
{
   int val1 = iexorop(line),val2;
	while (**line == '|' && (*line)[1] != '|') {
		++(*line);
		val2 = iexorop(line);
		val1 = val1 | val2;
	}
	return(val1);
}
static int ielandop(const char **line)
/* logical and op */
{
   int val1 = ieorop(line),val2;
	while (**line == '&' && (*line)[1] == '&') {
		*line += 2;
		val2 = ieorop(line);
		val1 = val1 && val2;
	}
	return(val1);
}
static int ielorop(const char **line)
/* logical or op */
{
   int val1 = ielandop(line),val2;
	while (**line == '|' && (*line)[1] == '|') {
		*line += 2;
		val2 = ielandop(line);
		val1 = val1 || val2;
	}
	return(val1);
}
static int iecondop(const char **line)
/* Hook op */
{
   int val1 = ielorop(line),val2, val3;
		if (**line == '?') {
			++(*line);
			val2 = iecondop(line);
			if (**line != ':')
				xprint( "preproc_eval: cond op missing :\n");
			else
				++(*line);
			val3 = iecondop(line);
			if (val1)
				val1 = val2;
			else
				val1 = val3;
		}
	return(val1);
}
int preproc_eval(const char **line)
/* Integer expressions */
{
	return iecondop(line);
}