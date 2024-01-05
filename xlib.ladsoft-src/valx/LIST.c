/* 
VALX linker
Copyright 1997-2011 David Lindauer.

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

Original 16-bit linker written by:
David Troendle

You may contact the author of this derivative at:
	mailto::camille@bluegrass.net
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <time.h>

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"
#include "globals.h"
/*                                 LIST.C                                  */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               ListDelete                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void ListDelete(Generic_Element_ptr       elem,
                Generic_Element_list     *lst)
BeginDeclarations
#define Elem                           (*elem)
#define Lst                            (*lst)
Generic_Element_ptr                    current;
#define Current                        (*current)
Generic_Element_ptr                    prior;
#define Prior                          (*prior)
EndDeclarations
BeginCode
 prior = Null;
 TraverseList(Lst, current)
  BeginTraverse
   If current Is elem
    Then
     If prior Is Null
      Then
       First(Lst) = Elem.next;
       If First(Lst) Is Null
        Then
         Last(Lst) = Null;
        EndIf;
      Else
       Prior.next = Elem.next;
       If Last(Lst) Is elem
        Then
         Last(Lst) = prior;
        EndIf;
      EndIf;
     ExitLoop;
    EndIf;
   prior = current;
  EndTraverse;
 return;
EndCode
#undef Elem
#undef Lst
#undef Current
#undef Prior

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           ListInsert                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void ListInsert(Generic_Element_ptr       elem,
                bit_16                    type_insert,
                Generic_Element_ptr       aftr,
                Generic_Element_list     *lst)
BeginDeclarations
#define Elem                           (*elem)
#define Aftr                           (*aftr)
#define Lst                            (*lst)
EndDeclarations
BeginCode
 Using type_insert
  BeginCase
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle:   Insert ptr After aftr InList lst EndInsert;                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
   When 0:
    Elem.next  = Aftr.next;
    Aftr.next  = elem;
    If Elem.next IsNull
     Then
      Last(Lst) = elem;
     EndIf;
    break;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle:   Insert ptr AtEnd InList lst EndInsert;                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
   When 1:
    If Last(Lst) IsNull
     Then
      First(Lst) = elem;
     Else
      (*Last(Lst)).next = elem;
     EndIf;
    Last(Lst)  = elem;
    Elem.next  = Null;
    break;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle:   Insert ptr AtBeginning InList lst EndInsert;                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
   When 2:
    Elem.next  = First(Lst);
    First(Lst) = elem;
    If Last(Lst) IsNull
     Then
      Last(Lst) = elem;
     EndIf;
    break;
  EndCase;
 return;
EndCode
#undef Elem
#undef Aftr
#undef Lst

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                ListPop                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void ListPop(Generic_Element_list     *lst,
             Generic_Element_ptr      *elem)
BeginDeclarations
#define Elem                           (*elem)
#define Lst                            (*lst)
EndDeclarations
BeginCode
 If First(Lst) IsNull
  Then
   Elem = Null;
   return;
  EndIf;
 Elem       = First(Lst);
 First(Lst) = (*Elem).next;
 If First(Lst) IsNull
  Then
   Last(Lst) = Null;
  EndIf;
 return;
EndCode
#undef Elem
#undef Lst

