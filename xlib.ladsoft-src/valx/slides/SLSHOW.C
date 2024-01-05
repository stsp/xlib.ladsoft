#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include "langext.h"
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                Constants                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
#define BLINK_ATTRIBUTE                0x80
#define COLUMNS_PER_ROW                80
#define COLUMNS_PER_TITLE              60
#define MAX_LINE_SIZE                  1024
#define MENU_ITEMS                     20
#define NORMAL_ATTRIBUTE               0x07
#define REVERSE_VIDEO                  0x70
#define ROWS_PER_MENU_HEADER           5
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
bit_16
       get_key                         (void);
bit_16
       get_slide                       (bit_16 slide_num);
bit_16
       get_title                       (bit_16 slide_num);
void
       main                            (bit_16 argc,
                                        char *argv[]);
bit_16
       menu                            (bit_16 slide_num);
void
       menu_header                     (void);
void
       menu_line                       (bit_16 line_num,
                                        int    video_attr);
void
       menu_page                       (bit_16 slide_num);
void
       show_slide                      (void);
char
      *string                          (char *s,
                                        bit_16 length);
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            Global Variables                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
union REGS                             dos_inregs;
union REGS                             dos_outregs;
bit_16                                 highest_slide_number_on_menu;
bit_16                                 lowest_slide_number_on_menu;
slide_type                             slide;
FILE                                  *slide_input_file;
char                                  *slide_input_file_name;
bit_16                                 slide_number;
char                                   temp_string[1024];
int                                    text_attribute;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                main                                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void main(bit_16 argc, char *argv[])
BeginDeclarations
bit_16                                 function;
bit_16                                 i;
EndDeclarations
BeginCode
 clrscr();
 If argc IsNot 2
  Then
   printf("Usage:\n");
   printf("\n");
   printf("  SLSHOW slideset\n");
   printf("\n");
   printf("    Where \"slideset\" is the name of a slide set file created\n");
   printf("    by the SLCOMP program.\n");
   printf("\n");
   exit(1);
  EndIf;
 slide_input_file_name = argv[1];
 slide_input_file = fopen(argv[1], "rb");
 If slide_input_file IsNull
  Then
   printf("Could not open file \"%s\" for input.\n", argv[1]);
   printf("\n");
   exit(4);
  EndIf;
 slide_number = menu(1);
 show_slide();
 function = get_key();
 Loop
  BeginLoop
   show_slide();
   Using function
    BeginCase
     When 0x0D:
     When 0x10D:  /* Enter */
      slide_number = menu(slide_number);
      show_slide();
      break;
     When 0x1B:   /* Esc */
      clrscr();
      break;
     When 0x148:  /* Cursor Up */
      If slide_number Is 1
       Then
        putch('\a');
       Else
        get_slide(slide_number-1);
        show_slide();
       EndIf;
      break;
     When 0x150: /* Cursor Down */
      i = slide_number;
      get_slide(slide_number+1);
      If slide_number IsZero
       Then
        get_slide(i);
        putch('\a');
       Else
        show_slide();
       EndIf;
      break;
     Otherwise:
      putch('\a');
      break;
    EndCase;
   ExitIf(function Is 0x1B);
   function = get_key();
   EndLoop;
 fclose(slide_input_file);
 exit(0);
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                get_key                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 get_key()
BeginDeclarations
bit_16                                 key;
EndDeclarations
BeginCode
 dos_inregs.h.ah = 0x08;
 intdos(Addr(dos_inregs), Addr(dos_outregs));
 If dos_outregs.h.al IsZero
  Then
   key = 256;
   intdos(Addr(dos_inregs), Addr(dos_outregs));
  Else
   key = 0;
  EndIf;
 key += Bit_16(dos_outregs.h.al);
 return(key);
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               get_slide                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 get_slide(bit_16 slide_num)
BeginDeclarations
bit_16                                 item_count;
bit_16                                 slide_found;
fpos_t                                 slide_position;
EndDeclarations
BeginCode
 slide_position = Bit_32(slide_num-1) * Bit_32(sizeof(slide_type));
 fsetpos(slide_input_file, Addr(slide_position));
 item_count = fread(Addr(slide), sizeof(slide_type), 1, slide_input_file);
 slide_found = item_count Is 1;
 If slide_found
  Then
   slide_number = slide_num;
  Else
   slide_number = 0;
  EndIf;
 return(slide_found);
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               get_title                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 get_title(bit_16 slide_num)
BeginDeclarations
bit_16                                 item_count;
bit_16                                 slide_found;
fpos_t                                 slide_position;
EndDeclarations
BeginCode
 slide_position = Bit_32(slide_num-1) * Bit_32(sizeof(slide_type));
 fsetpos(slide_input_file, Addr(slide_position));
 item_count = fread(Addr(slide), COLUMNS_PER_TITLE, 1, slide_input_file);
 slide_found = item_count Is 1;
 If slide_found
  Then
   slide_number = slide_num;
  Else
   slide_number = 0;
  EndIf;
 return(slide_found);
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                  menu                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 menu(bit_16 slide_num)
BeginDeclarations
bit_16                                 current_index;
int                                    function;
EndDeclarations
BeginCode
 menu_header();
 menu_page(slide_num);
 If lowest_slide_number_on_menu IsZero
  Then
   slide_num = 1;
   menu_page(slide_num);
   If lowest_slide_number_on_menu IsZero
    Then
     clrscr();
     printf("Error:  No slides in file \"%s\".\n", slide_input_file_name);
     exit(4);
    EndIf;
  EndIf;
 current_index = 0;
 function = get_key();
 Loop
  BeginLoop
   Using function
    BeginCase
     When 0x0D:
     When 0x10D:  /* Enter */
      break;
     When 0x148:  /* Cursor Up */
      If current_index IsZero
       Then
        If lowest_slide_number_on_menu Exceeds 1
         Then
          menu_page(lowest_slide_number_on_menu-1);
          current_index = 0;
         Else
          putch('\a');
         EndIf;
       Else
        get_title(lowest_slide_number_on_menu + current_index);
        menu_line(current_index+1, NORMAL_ATTRIBUTE);
        current_index--;
        get_title(lowest_slide_number_on_menu + current_index);
        menu_line(current_index+1, REVERSE_VIDEO);
        textattr(NORMAL_ATTRIBUTE);
       EndIf;
      break;
     When 0x149:  /* PgUp */
      If lowest_slide_number_on_menu Exceeds 1
       Then
        If lowest_slide_number_on_menu Exceeds MENU_ITEMS
         Then
          slide_num -= MENU_ITEMS;
         Else
          slide_num = 1;
         EndIf;
        menu_page(slide_num);
        current_index = 0;
       Else
        putch('\a');
       EndIf;
      break;
     When 0x150:  /* Cursor Down */
      If lowest_slide_number_on_menu + current_index Is
         highest_slide_number_on_menu
       Then
        get_title(highest_slide_number_on_menu+1);
        If slide_number IsNotZero
         Then
          slide_num = highest_slide_number_on_menu+1;
          menu_page(slide_num);
          current_index = 0;
         Else
          putch('\a');
         EndIf;
       Else
        get_title(lowest_slide_number_on_menu + current_index);
        menu_line(current_index+1, NORMAL_ATTRIBUTE);
        current_index++;
        get_title(lowest_slide_number_on_menu + current_index);
        menu_line(current_index+1, REVERSE_VIDEO);
       EndIf;
      break;
     When 0x151:  /* PgDn */
      get_title(highest_slide_number_on_menu+1);
      If slide_number IsNotZero
       Then
        slide_num = highest_slide_number_on_menu+1;
        menu_page(slide_num);
        current_index = 0;
       Else
        putch('\a');
       EndIf;
      break;
     Otherwise:
      putch('\a');
      break;
    EndCase;
   ExitIf((function Is 0x0D) OrIf (function Is 0x10D));
   function = get_key();
  EndLoop;
 get_slide(lowest_slide_number_on_menu + current_index);
 return(slide_number);
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              menu_header                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void menu_header()
BeginDeclarations
EndDeclarations
BeginCode
 clrscr();
 gotoxy(36,1);
 cprintf("Slide Menu");
 gotoxy(1,3);
 cprintf("File:  %s", slide_input_file_name);
 gotoxy(1, 5);
 cprintf("Slide Number  ------------------------Slide Title"
                       "-------------------------");
 return;
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               menu_line                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void menu_line(bit_16 line_num, int video_attr)
BeginDeclarations
EndDeclarations
BeginCode
 text_attribute = video_attr;
 textattr(text_attribute);
 gotoxy(1, line_num + ROWS_PER_MENU_HEADER);
 If slide_number IsZero
  Then
   clreol();
   return;
  EndIf;
 cprintf("    %3u       %s", slide_number,
                             string(slide.title, COLUMNS_PER_TITLE));
 return;
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               menu_page                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void menu_page(bit_16 slide_num)
BeginDeclarations
bit_16                                 i;
EndDeclarations
BeginCode
 lowest_slide_number_on_menu  =
 highest_slide_number_on_menu = 0;
 For i=0; i LessThan MENU_ITEMS; i++
  BeginFor
   get_title(slide_num+i);
   If slide_number IsNotZero
    Then
     If i IsZero
      Then
       lowest_slide_number_on_menu = slide_num;
      EndIf;
     highest_slide_number_on_menu = slide_num + i;
    EndIf;
   If i IsZero
    Then
     menu_line(i+1, REVERSE_VIDEO);
    Else
     menu_line(i+1, NORMAL_ATTRIBUTE);
    EndIf;
  EndFor;
 return;
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              show_slide                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void show_slide()
BeginDeclarations
bit_16                                 row;
EndDeclarations
BeginCode
 text_attribute = NORMAL_ATTRIBUTE;
 textattr(text_attribute);
 clrscr();
 For row=0; row LessThan ROWS_PER_PAGE; row++
  BeginFor
   gotoxy(1,row+1);
   cprintf("%s", string(slide.screen[row], COLUMNS_PER_ROW));
  EndFor;
 gotoxy(1, ROWS_PER_PAGE + 1);
 clreol();
 cprintf("Slide:  %u", slide_number);
 gotoxy(COLUMNS_PER_ROW - COLUMNS_PER_TITLE, ROWS_PER_PAGE + 1);
 cprintf("%s", string(slide.title, COLUMNS_PER_TITLE));
 return;
EndCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 string                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
char *string(char *s, bit_16 length)
BeginDeclarations
EndDeclarations
BeginCode
 memmove(temp_string, s, length);
 temp_string[length] = '\000';
 return(temp_string);
EndCode
