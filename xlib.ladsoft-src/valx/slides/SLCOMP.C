#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <conio.h>
#include <string.h>
#include "langext.h"
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                Constants                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
#define COLUMNS_PER_ROW                80
#define COLUMNS_PER_TITLE              60
#define MAX_LINE_SIZE                  1024
#define ROWS_PER_PAGE                  24
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 Types                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
Type char                              line_type[COLUMNS_PER_ROW];
Type line_type                         page_type[ROWS_PER_PAGE];
Structure slide_struct
 BeginStructure
  char                                 title[COLUMNS_PER_TITLE];
  page_type                            screen;
 EndStructure;
Type Structure slide_struct            slide_type;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               Prototypes                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
char
      *next_line                       (void);
void
       main                            (bit_16 argc, char *argv[]);
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            Global Variables                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
char                                   line[MAX_LINE_SIZE];
bit_16                                 line_count;
bit_16                                 line_length;
slide_type                             slide;
bit_16                                 slide_count;
FILE                                  *slide_input_file;
FILE                                  *slide_output_file;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                main                                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void main(bit_16 argc, char *argv[])
BeginDeclarations
char                                  *text;
EndDeclarations
BeginCode
 clrscr();
 If argc IsNot 3
  Then
   printf("Usage:\n");
   printf("\n");
   printf("  SLCOMP infile outfile\n");
   printf("\n");
   printf("    Where \"infile\" is the slide input created by an editor\n");
   printf("    and \"outfile\" is the compiled output filename suitable\n");
   printf("    for viewing by the SLSHOW program.\n");
   exit(1);
  EndIf;
 slide_input_file = fopen(argv[1], "rt");
 If slide_input_file IsNull
  Then
   printf("Could not open file \"%s\" for input.\n", argv[1]);
   printf("\n");
   exit(4);
  EndIf;
 slide_output_file = fopen(argv[2], "wb");
 If slide_output_file IsNull
  Then
   printf("Could not open file \"%s\" for output.\n", argv[2]);
   printf("\n");
   exit(4);
  EndIf;
 text = next_line();
 If (text IsNull) OrIf (text[0] IsNot '\f')
  Then
   clrscr();
   printf("Error:  Slide set does not start with a title.\n");
   printf("\n");
   exit(4);
  EndIf;
 While text[0] Is '\f'
  BeginWhile
   slide_count++;
   setmem(Addr(slide), sizeof(slide_type), ' ');
   text = Addr(line[1]);
   line_length--;
   If line_length Exceeds COLUMNS_PER_TITLE
    Then
     line_length = COLUMNS_PER_TITLE;
    EndIf;
   text[line_length] = '\000';
   memmove(slide.title, text, line_length);
   printf("Slide:  %u,  Title:  \"%s\"\n", slide_count, text);
   line_count = 0;
   text = next_line();
   While (text IsNotNull) AndIf (text[0] IsNot '\f')
    BeginWhile
     line_count++;
     If line_count Is ROWS_PER_PAGE+1
      Then
       printf("  Warning:  Slide exceeds %u lines.\n", ROWS_PER_PAGE);
      EndIf;
     If line_count NotGreaterThan ROWS_PER_PAGE
      Then
       If line_length Exceeds COLUMNS_PER_ROW
        Then
         printf("  Warning:  Line %u exceeds %u characters.\n",
                line_count, COLUMNS_PER_ROW);
         line_length = COLUMNS_PER_ROW;
        EndIf;
       memmove(slide.screen[line_count-1], text, line_length);
      EndIf;
     text = next_line();
    EndWhile;
   fwrite(Addr(slide), sizeof(slide_type), 1, slide_output_file);
  EndWhile;
 printf("\n");
 fclose(slide_input_file);
 fclose(slide_output_file);
 exit(0);
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               next_line                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
char *next_line()
BeginDeclarations
char                                  *text;
EndDeclarations
BeginCode
 text = fgets(line, MAX_LINE_SIZE, slide_input_file);
 If text IsNull
  Then
   line_length = 0;
   return(text);
  EndIf;
 line_length = Bit_16(strlen(line));
 If line[--line_length] IsNot '\n'
  Then
   printf("Error:  A line exceeds %u characters.\n", MAX_LINE_SIZE);
   exit(4);
  EndIf;
 line[line_length] = '\000';
 return(text);
EndCode



