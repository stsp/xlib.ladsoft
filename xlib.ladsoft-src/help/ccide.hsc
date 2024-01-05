HelpScribble project file.
16
YNQ`bsg-113363
0
2
CCIDE Help


LADSoft
TRUE

c:\tools\cc\help\pictures,C:\tools\cc\help\pictures\menu,C:\tools\cc\help\pictures\dialog,C:\tools\cc\help\pictures\contextmenu,C:\tools\cc\help\pictures\wnd,C:\tools\cc\help\pictures\generalproperties,c:\tools\cc\help\pictures\toolbar,c:\tools\cc\help\pictures\toolproperties
1
BrowseButtons()
0
TRUE

FALSE
TRUE
16777215
0
32768
8388736
255
TRUE
TRUE
TRUE
FALSE
150
50
600
500
TRUE
FALSE
1
FALSE
FALSE
Contents
%s Contents
Index
%s Index
Previous
Next
FALSE

91
10
Scribble10
index
index;title;



Writing



FALSE
34
{\rtf1\ansi\ansicpg1252\deff0{\fonttbl{\f0\fnil\fcharset0 Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\lang1033\b\fs32 Welcome to CCIDE help
\par 
\par \cf0\b0\fs20 CCIDE is an integrated development environment tailored to use with the CC386 tool set.  It supports most functionality of the tool set; however it is not a standalone program.  To accomplish its tasks it usually spawns an appropriate tool from the tool set, such as the compiler or linker.  Output from spawned tools is integrated into the information window.
\par 
\par CCIDE may also be used as a standalone editor.
\par 
\par The following topics contain general information to help you get started:
\par 
\par \tab\cf2\strike Getting Started\cf3\strike0\{linkID=20\}
\par \tab\cf2\strike Support For NASM\cf3\strike0\{linkID=840\}
\par \cf0 
\par CCIDE has the following general functions:
\par 
\par \tab\cf2\strike Configurable Integrated editor\cf3\strike0\{linkID=50\}\cf0 
\par \tab\cf2\strike Project window, with integrated make facility\cf3\strike0\{linkID=190\}\cf0 
\par \tab\cf2\strike Information windows\cf3\strike0\{linkID=550\}\cf0 
\par \tab\cf2\strike Integrated debugger\cf3\strike0\{linkID=300\}\cf0 
\par \tab\cf2\strike Debugging windows\cf3\strike0\{linkID=320\}\cf0 
\par \tab\cf2\strike Find and Replace\cf3\strike0\{linkID=100\}
\par \cf0\tab\cf2\strike Bookmarks\cf3\strike0\{linkID=400\}\cf0 
\par \tab\cf2\strike Configurable platform help\cf3\strike0\{linkID=420\}\cf0 
\par \tab\cf2\strike Browse information\cf3\strike0\{linkID=430\}
\par \tab\cf2\strike Printing\cf3\strike0\{linkID=440\}
\par \tab\cf2\strike Menus\cf3\strike0\{linkID=40\}
\par \tab\cf2\strike Toolbars\cf3\strike0\{linkID=450\}
\par \tab\cf2\strike Hot keys\cf3\strike0\{linkID=850\}
\par \tab\cf2\strike General Properties Dialog\cf3\strike0\{linkID=550\}
\par \tab\cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}
\par  
\par \cf1\b\fs32 For licensing information, look here:
\par \tab\cf2\b0\strike\fs20 GPL Copyright Information\cf3\strike0\{linkID=30\}\cf2\strike 
\par }
20
Scribble20
Getting Started
begin;getting started;start;



Writing



FALSE
29
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}{\f2\fnil\fcharset0 Courier New;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\b\fs32 Getting Started\cf0\b0\f1\fs20 
\par 
\par \f0 CCIDE is a \cf2\strike workarea\cf3\strike0\{linkID=190\}\cf0  - based system.  That means to do anything significant other than simply \cf2\strike editing\cf3\strike0\{linkID=50\}\cf0  files (such as \cf2\strike compiling\cf3\strike0\{linkID=250\}\cf0  or \cf2\strike debugging\cf3\strike0\{linkID=300\}\cf0 ) a workarea must first be set up.  The workarea has information about \b Projects\b0 , which are executable or DLL files that are to be created.  A list of sources files is associated with each project; when attempting to create the project the IDE will apply the appropriate tool to each source file to compile it, then link the results together to create the project.  CCIDE maintains a list of dependency files and automatically rebuild all related source files when one of the dependency files changes.
\par 
\par The list of source files may be any combination of C language files, Assembly language files, RC files, and libraries.  CCIDE will automatically apply the compiler, the assembler, the resource compiler, or the linker depending on the file name extension of the source file.  If a \b Make\b0  option is selected, only files which have changed, or whose dependencies have changed, will be recompiled.  If the \b Build All\b0  option is selected CCIDE will rebuild all files irregardless of what has changed.
\par 
\par Creating a Workarea is a four-step process.  First, select\b  File -> Work Area -> New Workarea\b0  from the menu bar.  An \cf2\strike Open File Dialog\cf3\strike0\{linkID=350\}\cf0   prompts for the name of the workarea.  By default, this dialog will want to save your workarea in a new subdirectory under the \b My Documents\b0  folder, but this location can be changed.
\par 
\par After creating the Workarea, a project must be created.  To create a project select the \b File -> Work Area -> Add -> New Project\b0  menu item from the menu bar.  A \cf2\strike New Project Dialog\cf3\strike0\{linkID=360\}\cf0  prompts for the name and type of the project.  Type in the name of a project and select its type.  The project name should not have a file extension, as one will be added based on the type you select.  Normally, one would select a \b Console\b0  project for standard C programming, or text-based programs that are meant to be be accessed from a command prompt.  \b Windows GUI\b0  may be selected to make a program that is intended to have a windowing user interface.  The other project types are used for sharing code between projects.
\par 
\par Once the project is created, right-clicking on the project's name or on the source header will cause the \cf2\strike Project Context Menu\cf3\strike0\{linkID=230\}\cf0   to appear.   Press \b Add -> New File\b0  to create a new file to type in.  Later, when you attempt to save the file an Open File Dialog prompts you for the name to save it as.
\par 
\par If this is your first program and you have selected \b Console\b0  as the program type you might try typing something like:
\par 
\par \f2 #include <stdio.h>
\par 
\par int main(int argc, char *argv[])
\par \{
\par \tab printf("Hello World");
\par \}
\par \f0 
\par This short program is a classic first program that simply displays the text \cf1 Hello World\cf0  when you run it.
\par 
\par Selecting \b File -> Work Area -> Save\b0  from the menu bar will ensure the workarea and project is saved to disk.
\par 
\par At this point the project may be built by selecting one of the items on the \cf2\strike Build Menu\cf3\strike0\{linkID=250\}.\cf0   Once it is built it may be debugged by starting the \cf2\strike Integrated Debugger\cf3\strike0\{linkID=300\}\cf0  from either the \cf2\strike Debug Menu\cf3\strike0\{linkID=310\}\cf0  or the \cf2\strike Debug Toolbar\cf3\strike0\{linkID=760\}\cf0 .\f1 
\par }
30
Scribble30
Copyright
copyright;



Writing



FALSE
358
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;}
\viewkind4\uc1\pard\fi540\cf1\fs20  \cf2                    GNU GENERAL PUBLIC LICENSE
\par                        Version 2, June 1991
\par 
\par  Copyright (C) 1989, 1991 Free Software Foundation, Inc.
\par      59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
\par  Everyone is permitted to copy and distribute verbatim copies
\par  of this license document, but changing it is not allowed.
\par 
\par                             Preamble
\par 
\par   The licenses for most software are designed to take away your
\par freedom to share and change it.  By contrast, the GNU General Public
\par License is intended to guarantee your freedom to share and change free
\par software--to make sure the software is free for all its users.  This
\par General Public License applies to most of the Free Software
\par Foundation's software and to any other program whose authors commit to
\par using it.  (Some other Free Software Foundation software is covered by
\par the GNU Library General Public License instead.)  You can apply it to
\par your programs, too.
\par 
\par   When we speak of free software, we are referring to freedom, not
\par price.  Our General Public Licenses are designed to make sure that you
\par have the freedom to distribute copies of free software (and charge for
\par this service if you wish), that you receive source code or can get it
\par if you want it, that you can change the software or use pieces of it
\par in new free programs; and that you know you can do these things.
\par 
\par   To protect your rights, we need to make restrictions that forbid
\par anyone to deny you these rights or to ask you to surrender the rights.
\par These restrictions translate to certain responsibilities for you if you
\par distribute copies of the software, or if you modify it.
\par 
\par   For example, if you distribute copies of such a program, whether
\par gratis or for a fee, you must give the recipients all the rights that
\par you have.  You must make sure that they, too, receive or can get the
\par source code.  And you must show them these terms so they know their
\par rights.
\par 
\par   We protect your rights with two steps: (1) copyright the software, and
\par (2) offer you this license which gives you legal permission to copy,
\par distribute and/or modify the software.
\par 
\par   Also, for each author's protection and ours, we want to make certain
\par that everyone understands that there is no warranty for this free
\par software.  If the software is modified by someone else and passed on, we
\par want its recipients to know that what they have is not the original, so
\par that any problems introduced by others will not reflect on the original
\par authors' reputations.
\par 
\par   Finally, any free program is threatened constantly by software
\par patents.  We wish to avoid the danger that redistributors of a free
\par program will individually obtain patent licenses, in effect making the
\par program proprietary.  To prevent this, we have made it clear that any
\par patent must be licensed for everyone's free use or not licensed at all.
\par 
\par   The precise terms and conditions for copying, distribution and
\par modification follow.
\par 
\par 
\par                     GNU GENERAL PUBLIC LICENSE
\par    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
\par 
\par   0. This License applies to any program or other work which contains
\par a notice placed by the copyright holder saying it may be distributed
\par under the terms of this General Public License.  The "Program", below,
\par refers to any such program or work, and a "work based on the Program"
\par means either the Program or any derivative work under copyright law:
\par that is to say, a work containing the Program or a portion of it,
\par either verbatim or with modifications and/or translated into another
\par language.  (Hereinafter, translation is included without limitation in
\par the term "modification".)  Each licensee is addressed as "you".
\par 
\par Activities other than copying, distribution and modification are not
\par covered by this License; they are outside its scope.  The act of
\par running the Program is not restricted, and the output from the Program
\par is covered only if its contents constitute a work based on the
\par Program (independent of having been made by running the Program).
\par Whether that is true depends on what the Program does.
\par 
\par 
\par 
\par 
\par 
\par 
\par 
\par 
\par 
\par 
\par   1. You may copy and distribute verbatim copies of the Program's
\par source code as you receive it, in any medium, provided that you
\par conspicuously and appropriately publish on each copy an appropriate
\par copyright notice and disclaimer of warranty; keep intact all the
\par notices that refer to this License and to the absence of any warranty;
\par and give any other recipients of the Program a copy of this License
\par along with the Program.
\par 
\par You may charge a fee for the physical act of transferring a copy, and
\par you may at your option offer warranty protection in exchange for a fee.
\par 
\par   2. You may modify your copy or copies of the Program or any portion
\par of it, thus forming a work based on the Program, and copy and
\par distribute such modifications or work under the terms of Section 1
\par above, provided that you also meet all of these conditions:
\par 
\par     a) You must cause the modified files to carry prominent notices
\par     stating that you changed the files and the date of any change.
\par 
\par     b) You must cause any work that you distribute or publish, that in
\par     whole or in part contains or is derived from the Program or any
\par     part thereof, to be licensed as a whole at no charge to all third
\par     parties under the terms of this License.
\par 
\par     c) If the modified program normally reads commands interactively
\par     when run, you must cause it, when started running for such
\par     interactive use in the most ordinary way, to print or display an
\par     announcement including an appropriate copyright notice and a
\par     notice that there is no warranty (or else, saying that you provide
\par     a warranty) and that users may redistribute the program under
\par     these conditions, and telling the user how to view a copy of this
\par     License.  (Exception: if the Program itself is interactive but
\par     does not normally print such an announcement, your work based on
\par     the Program is not required to print an announcement.)
\par 
\par 
\par These requirements apply to the modified work as a whole.  If
\par identifiable sections of that work are not derived from the Program,
\par and can be reasonably considered independent and separate works in
\par themselves, then this License, and its terms, do not apply to those
\par sections when you distribute them as separate works.  But when you
\par distribute the same sections as part of a whole which is a work based
\par on the Program, the distribution of the whole must be on the terms of
\par this License, whose permissions for other licensees extend to the
\par entire whole, and thus to each and every part regardless of who wrote it.
\par 
\par Thus, it is not the intent of this section to claim rights or contest
\par your rights to work written entirely by you; rather, the intent is to
\par exercise the right to control the distribution of derivative or
\par collective works based on the Program.
\par 
\par In addition, mere aggregation of another work not based on the Program
\par with the Program (or with a work based on the Program) on a volume of
\par a storage or distribution medium does not bring the other work under
\par the scope of this License.
\par 
\par   3. You may copy and distribute the Program (or a work based on it,
\par under Section 2) in object code or executable form under the terms of
\par Sections 1 and 2 above provided that you also do one of the following:
\par 
\par     a) Accompany it with the complete corresponding machine-readable
\par     source code, which must be distributed under the terms of Sections
\par     1 and 2 above on a medium customarily used for software interchange; or,
\par 
\par     b) Accompany it with a written offer, valid for at least three
\par     years, to give any third party, for a charge no more than your
\par     cost of physically performing source distribution, a complete
\par     machine-readable copy of the corresponding source code, to be
\par     distributed under the terms of Sections 1 and 2 above on a medium
\par     customarily used for software interchange; or,
\par 
\par     c) Accompany it with the information you received as to the offer
\par     to distribute corresponding source code.  (This alternative is
\par     allowed only for noncommercial distribution and only if you
\par     received the program in object code or executable form with such
\par     an offer, in accord with Subsection b above.)
\par 
\par The source code for a work means the preferred form of the work for
\par making modifications to it.  For an executable work, complete source
\par code means all the source code for all modules it contains, plus any
\par associated interface definition files, plus the scripts used to
\par control compilation and installation of the executable.  However, as a
\par special exception, the source code distributed need not include
\par anything that is normally distributed (in either source or binary
\par form) with the major components (compiler, kernel, and so on) of the
\par operating system on which the executable runs, unless that component
\par itself accompanies the executable.
\par 
\par If distribution of executable or object code is made by offering
\par access to copy from a designated place, then offering equivalent
\par access to copy the source code from the same place counts as
\par distribution of the source code, even though third parties are not
\par compelled to copy the source along with the object code.
\par 
\par 
\par   4. You may not copy, modify, sublicense, or distribute the Program
\par except as expressly provided under this License.  Any attempt
\par otherwise to copy, modify, sublicense or distribute the Program is
\par void, and will automatically terminate your rights under this License.
\par However, parties who have received copies, or rights, from you under
\par this License will not have their licenses terminated so long as such
\par parties remain in full compliance.
\par 
\par   5. You are not required to accept this License, since you have not
\par signed it.  However, nothing else grants you permission to modify or
\par distribute the Program or its derivative works.  These actions are
\par prohibited by law if you do not accept this License.  Therefore, by
\par modifying or distributing the Program (or any work based on the
\par Program), you indicate your acceptance of this License to do so, and
\par all its terms and conditions for copying, distributing or modifying
\par the Program or works based on it.
\par 
\par   6. Each time you redistribute the Program (or any work based on the
\par Program), the recipient automatically receives a license from the
\par original licensor to copy, distribute or modify the Program subject to
\par these terms and conditions.  You may not impose any further
\par restrictions on the recipients' exercise of the rights granted herein.
\par You are not responsible for enforcing compliance by third parties to
\par this License.
\par 
\par   7. If, as a consequence of a court judgment or allegation of patent
\par infringement or for any other reason (not limited to patent issues),
\par conditions are imposed on you (whether by court order, agreement or
\par otherwise) that contradict the conditions of this License, they do not
\par excuse you from the conditions of this License.  If you cannot
\par distribute so as to satisfy simultaneously your obligations under this
\par License and any other pertinent obligations, then as a consequence you
\par may not distribute the Program at all.  For example, if a patent
\par license would not permit royalty-free redistribution of the Program by
\par all those who receive copies directly or indirectly through you, then
\par the only way you could satisfy both it and this License would be to
\par refrain entirely from distribution of the Program.
\par 
\par If any portion of this section is held invalid or unenforceable under
\par any particular circumstance, the balance of the section is intended to
\par apply and the section as a whole is intended to apply in other
\par circumstances.
\par 
\par It is not the purpose of this section to induce you to infringe any
\par patents or other property right claims or to contest validity of any
\par such claims; this section has the sole purpose of protecting the
\par integrity of the free software distribution system, which is
\par implemented by public license practices.  Many people have made
\par generous contributions to the wide range of software distributed
\par through that system in reliance on consistent application of that
\par system; it is up to the author/donor to decide if he or she is willing
\par to distribute software through any other system and a licensee cannot
\par impose that choice.
\par 
\par This section is intended to make thoroughly clear what is believed to
\par be a consequence of the rest of this License.
\par 
\par 
\par   8. If the distribution and/or use of the Program is restricted in
\par certain countries either by patents or by copyrighted interfaces, the
\par original copyright holder who places the Program under this License
\par may add an explicit geographical distribution limitation excluding
\par those countries, so that distribution is permitted only in or among
\par countries not thus excluded.  In such case, this License incorporates
\par the limitation as if written in the body of this License.
\par 
\par   9. The Free Software Foundation may publish revised and/or new versions
\par of the General Public License from time to time.  Such new versions will
\par be similar in spirit to the present version, but may differ in detail to
\par address new problems or concerns.
\par 
\par Each version is given a distinguishing version number.  If the Program
\par specifies a version number of this License which applies to it and "any
\par later version", you have the option of following the terms and conditions
\par either of that version or of any later version published by the Free
\par Software Foundation.  If the Program does not specify a version number of
\par this License, you may choose any version ever published by the Free Software
\par Foundation.
\par 
\par   10. If you wish to incorporate parts of the Program into other free
\par programs whose distribution conditions are different, write to the author
\par to ask for permission.  For software which is copyrighted by the Free
\par Software Foundation, write to the Free Software Foundation; we sometimes
\par make exceptions for this.  Our decision will be guided by the two goals
\par of preserving the free status of all derivatives of our free software and
\par of promoting the sharing and reuse of software generally.
\par 
\par                             NO WARRANTY
\par 
\par   11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
\par FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
\par OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
\par PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
\par OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
\par MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
\par TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
\par PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
\par REPAIR OR CORRECTION.
\par 
\par   12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
\par WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
\par REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
\par INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
\par OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
\par TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
\par YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
\par PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
\par POSSIBILITY OF SUCH DAMAGES.
\par 
\par                      END OF TERMS AND CONDITIONS
\par 
\par 
\par             How to Apply These Terms to Your New Programs
\par 
\par   If you develop a new program, and you want it to be of the greatest
\par possible use to the public, the best way to achieve this is to make it
\par free software which everyone can redistribute and change under these terms.
\par 
\par   To do so, attach the following notices to the program.  It is safest
\par to attach them to the start of each source file to most effectively
\par convey the exclusion of warranty; and each file should have at least
\par the "copyright" line and a pointer to where the full notice is found.
\par 
\par     <one line to give the program's name and a brief idea of what it does.>
\par \pard     Copyright (C) 19yy  <name of author>
\par 
\par     This program is free software; you can redistribute it and/or modify
\par     it under the terms of the GNU General Public License as published by
\par     the Free Software Foundation; either version 2 of the License, or
\par     (at your option) any later version.
\par 
\par     This program is distributed in the hope that it will be useful,
\par     but WITHOUT ANY WARRANTY; without even the implied warranty of
\par     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
\par     GNU General Public License for more details.
\par 
\par     You should have received a copy of the GNU General Public License
\par     along with this program; if not, write to the Free Software
\par     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
\par 
\par 
\par Also add information on how to contact you by electronic and paper mail.
\par 
\par If the program is interactive, make it output a short notice like this
\par when it starts in an interactive mode:
\par 
\par     Gnomovision version 69, Copyright (C) 19yy name of author
\par     Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
\par     This is free software, and you are welcome to redistribute it
\par     under certain conditions; type `show c' for details.
\par 
\par The hypothetical commands `show w' and `show c' should show the appropriate
\par parts of the General Public License.  Of course, the commands you use may
\par be called something other than `show w' and `show c'; they could even be
\par mouse-clicks or menu items--whatever suits your program.
\par 
\par You should also get your employer (if you work as a programmer) or your
\par school, if any, to sign a "copyright disclaimer" for the program, if
\par necessary.  Here is a sample; alter the names:
\par 
\par   Yoyodyne, Inc., hereby disclaims all copyright interest in the program
\par   `Gnomovision' (which makes passes at compilers) written by James Hacker.
\par 
\par   <signature of Ty Coon>, 1 April 1989
\par   Ty Coon, President of Vice
\par 
\par This General Public License does not permit incorporating your program into
\par proprietary programs.  If your program is a subroutine library, you may
\par consider it more useful to permit linking proprietary applications with the
\par library.  If this is what you want to do, use the GNU Library General
\par Public License instead of this License.
\par \cf0\f1 
\par }
40
Scribble40
Menus
menu;



Writing



FALSE
37
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Menus\cf2\b0\f1\fs20 
\par 
\par \f0 The IDE has two types of menus.  The first type is the menu bar at the top of the program's window interface.  The second type is context-sensitive menus that are available when right-clicking in the area of various special windows.
\par 
\par The menu bar at the top of the program appears as follows:
\par 
\par \pard\qc\cf3\{bmc menubar.png\}\cf2 
\par 
\par \pard It has the following menus:
\par 
\par \pard\tx700\tx4320\cf4\tab\strike File menu\cf3\strike0\{linkID=60\}\tab\cf2 Shows file, workarea, and general menu items\cf3 
\par \cf4\tab\strike Edit Menu\cf3\strike0\{linkID=130\}\tab\cf0 Shows items related to editing\cf2 
\par \cf4\tab\strike\f1 View Menu\cf3\strike0\{linkID=180\}\f0\tab\cf0 Shows the states of special windows, browse information, and bookmarks\cf3\f1 
\par \cf4\f0\tab\strike Search Menu\cf3\strike0\{linkID=90\}\tab\cf0 Shows options related to searching for text\cf4 
\par \tab\strike Project Menu\cf3\strike0\{linkID=110\}\cf0\tab Shows options related to the active project\cf4 
\par \tab\strike\f1 Build Menu\cf3\strike0\{linkID=250\}\f0\tab\cf0 Shows items related to compiling and linking programs\cf3\f1 
\par \cf4\f0\tab\strike\f1 Debug Menu\cf3\strike0\{linkID=310\}\cf0\f0\tab Shows items related to debugging
\par \tab\cf4\strike Tools Menu\cf3\strike0\{linkID=120\}\cf0\tab Shows user defined tools and items related to customizing the IDE\cf3\f1 
\par \cf4\f0\tab\strike\f1 Window Menu\cf3\strike0\{linkID=140\}\f0\tab\cf0 Shows items related to window management\cf3\f1 
\par \cf4\f0\tab\strike\f1 Help Menu\cf3\strike0\{linkID=260\}\f0\tab\cf0 Shows items related to documentation
\par 
\par 
\par \pard The following context-sensitive menus are available by right-clicking on various windows:\cf2\f1 
\par 
\par \pard\tx680\tx4300\f0\tab\cf4\strike Client Context Menu\cf3\strike0\{linkID=200\}\tab\cf2 Shows items related to managing editor windows\f1 
\par \cf4\f0\tab\strike\f1 Editor Context Menu\cf3\strike0\{linkID=150\}\cf0\f0\tab Shows items related to editing, bookmarks, and debugging\cf3\f1 
\par \cf4\f0\tab\strike\f1 Information Context Menu\cf3\strike0\{linkID=290\}\cf0\f0\tab Shows items related to compiling and linking programs\cf3\f1 
\par \cf4\f0\tab\strike\f1 Watch Context Menu\cf3\strike0\{linkID=660\}\cf0\f0\tab Shows items related to the watch window
\par \tab\cf4\strike Memory Context Menu\cf3\strike0\{linkID=690\}\cf0\tab Shows items relates to size of memory units\cf3\f1 
\par \cf4\f0\tab\strike\f1 Work\f0 area\f1  Context Menu\cf3\strike0\{linkID=210\}\cf0\f0\tab Shows items related to the workarea\cf3\f1 
\par \f0\tab\cf4\strike Project Context Menu\cf3\strike0\{linkID=230\}\cf0\tab Shows Items related to the selected project\cf3 
\par \tab\cf4\strike Folder Context Menu\cf3\strike0\{linkID=220\}\cf0\tab\tab Shows items related to the selected folder\cf3 
\par \tab\cf4\strike File Context Menu\cf3\strike0\{linkID=240\}\cf0\tab Shows items related to the selected file\cf3\f1 
\par \cf2 
\par }
50
Scribble50
Editor Window
editor window;windows;



Writing



FALSE
26
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Editor Window\cf0\b0\f1\fs20 
\par 
\par 
\par \pard\ri1440\tx660\tx2820\f0 The editor is a colorizing editor with basic functionality similar to a rich edit control, and various extended functionality added to support various IDE functionality.  It can colorize for the C language, NASM, and Resource files.  Colorization is done based on file extension.  \cf2\strike Configuration\cf3\strike0\{linkID=550\}\cf0  of basic functionality is supported.  An example follows.
\par 
\par \pard\ri1440\qc\tx660\tx2820\cf3\{bmc debugsession.png\}\cf0 
\par \pard\ri1440\tx660\tx2820 
\par In this example, the \cf2\strike Integrated Debugger\cf3\strike0\{linkID=300\}\cf0  has been started.  The dark grey bars to the left of the text show code lines which have generated code associated with them, and are suitable for breakpoints.  The red circle is a break point.  The yellow arrow shows the position at which the program flow was stopped.  This example also shows a bookmark, which is a grey square.
\par 
\par When one or more files are opened for editing, a series of tabs at the top of the client area will show a list of all open files, and allow easy navigation to a given file.  If there are two many open files, an arrow button to the far right of the tabs will allow opening a menu to select from available files.  A star next to a file name indicates the file has changed and needs to be saved.
\par 
\par Above all the editor windows but below the tab with the file names is the \cf2\strike Jump List\cf3\strike0\{linkID=900\}\cf0 .  The box at the left selects a type of variable such as a global functions or global variables, and the box at the right gives a list of function or variable names.  Selecting a function or variable name from the box at the right opens a window for the associated file (if it isn't already open) and positions the cursor on the line where the variable or function is declared.
\par 
\par Other ways to navigate to an open window is to view the window list on the \cf2\strike Window menu\cf3\strike0\{linkID=140\}\cf0 , and to use the \cf2\strike Client Context Menu\cf3\strike0\{linkID=200\}\cf0  to open the \cf2\strike Select Window\cf3\strike0\{linkID=330\}\cf0  dialog.
\par 
\par Various menu items are available to help manage the editor windows.  These include items on the \cf2\strike File menu\cf3\strike0\{linkID=60\}\cf0  of the main menu, on the \cf2\strike Edit menu\cf3\strike0\{linkID=130\}\cf0  of the main menu, the Windows menu, the Client Context Menu, and the \cf2\strike Context menu\cf3\strike0\{linkID=150\}\cf0  available in edit windows.
\par \pard\fi-2780\li2780\ri1440\tx680\tx2820 
\par \pard\ri1440\tx660\tx2820 Double clicking in the area to the left of the window will toggle a \cf2\strike Breakpoint\cf3\strike0\{linkID=410\}\cf0  wit the selected line.
\par 
\par Pressing SHIFT-F1 while an edit window is selected will bring up C runtime library help for the word under the cursor.
\par \pard\fi-2780\li2780\ri1440\tx680\tx2820 
\par \pard\ri1440\tx660\tx2820 In the background, the IDE will parse through files that are in open edit windows, to get lists of type information and structure elements.  This information is used for
\par code completion, and to show hints about variable types.
\par }
60
Scribble60
File Menu
file;file menu;menu;



Writing



FALSE
22
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 File Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The File Menu has menu items related to files and some generic commands.  These include the file and workarea submenus.
\par \pard\qc 
\par \cf3\{bmc mfile.png\}\cf2 
\par \pard 
\par \b Work Area\b0  is used to access \cf4\strike workarea related\cf3\strike0\{linkID=70\}\cf2  features
\par 
\par \b File\b0  is used to access \cf4\strike file related\cf3\strike0\{linkID=80\}\cf2  features
\par 
\par \b Save All Files\b0  is used to save changes to all open files
\par 
\par \b Print\b0  is used to \cf4\strike print\cf3\strike0\{linkID=440\}\cf2  files. It opens the standard windows printer dialog and allows selection of various properties related to the print request.
\par 
\par \b Command Window \b0 opens a command prompt.  The command prompt will have sufficient environment variables defined to build the project.
\par 
\par \b Exit\b0  exits the IDE.  If any files need to be saved or the debugger needs to be closed, a prompt will appear requesting confirmation.
\par 
\par 
\par }
70
Scribble70
Workarea Menu
menu;new;new menu;



Writing



FALSE
33
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Workarea Menus\cf2\b0\f1\fs20 
\par 
\par \f0 The Workarea Menus allow creating and opening workspaces, and adding projects.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc mworkarea.png\}
\par \pard 
\par \cf0\b New\b0  brings up the \cf4\strike Open File Dialog\cf3\strike0\{linkID=350\}\cf0  to get the name of a workarea to create.
\par 
\par \b Open\b0  brings up the \cf4\strike Open File Dialog\cf3\strike0\{linkID=350\}\cf0  to get the name of an existing workarea to reopen.
\par 
\par \b Reopen\b0  shows a menu that has recently selected workareas; selecting one opens it.
\par 
\par \b Save\b0  saves the workarea and any projects
\par 
\par \b Close\b0  closes the workarea and returns to the default
\par 
\par \b Import\b0  is a subment that allows importing projects from earlier versions of the IDE.  It has the following options:
\par 
\par \pard\fi1120\b Old Workspace\b0  converts an old workspace to the new format and imports it
\par 
\par \b Old Target\b0  converts an old target(project) file to the new format and imports it into the current workspace.
\par 
\par \pard\li1100 Note that when importing an old workspace, the old workspace may override some features such as the new toolbars.  Such features can be reenabled from the menus.
\par \pard\b 
\par Add\b0  is a submenu.  It has the following options
\par 
\par \pard\fi1140\b New Project\b0  brings up the\cf4\strike  New Project Dialog\cf3\strike0\{linkID=360\}\cf0  to get the name of a new project to open.
\par 
\par \b Existing Project \b0 brings up the \cf4\strike Open File Dialog\cf3\strike0\{linkID=350\}\cf0  to get the name of an existing project to reopen.
\par \b 
\par }
80
Scribble80
FileFile Menu




Writing



FALSE
21
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 File Submenu\cf0\b0\f1\fs20 
\par 
\par \f0 The File Submenu allows various operations associated with source code files.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc mfilefile.png\}\cf0 
\par \pard 
\par 
\par \b New\b0  creates a new file.  Initially the file will be untitled.  Attempting to save it will bring up a variation of the \cf3\strike Open File Dialog\cf2\strike0\{linkID=350\}\cf0  to select its name.
\par 
\par \b Open\b0  brings up the Open File Dialog to get the name of the file to open.
\par 
\par \b Reopen\b0  shows a menu that has recently selected files; selecting one opens it.
\par 
\par \b Save\b0  saves any changes made to a file.
\par 
\par \b SaveAs\b0  opens a variation of the Open File Dialog to allow selection of a new file name to save th file under.
\par 
\par \b Close\b0  closes the window associated with a file.\f1 
\par }
90
Scribble90
Search Menu




Writing



FALSE
17
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Search Menu\cf0\b0\f1\fs20 
\par 
\par \f0 The Search Menu shows options related to searching for text, and replacing one text with another.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc msearch.png\}\cf0 
\par \pard 
\par \b Find\b0  brings up a \cf3\strike Find/Replace\cf2\strike0\{linkID=100\}\cf0  dialog configured for locating text in editor windows.
\par \b 
\par Replace\b0  brings up a Find/Replace dialog configured for replacing text in editor windows.
\par 
\par \b Find Next\b0  locates the next occurrance of the last text searched for.
\par 
\par \b Find In Files\b0  brings up a Find/Replace dialog configured for locating text in files that are not in editor windows.
\par \f1 
\par }
100
IDH_FIND_REPLACE_DIALOG
Find/Replace dialog




Writing



FALSE
95
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Find/Replace Dialog\cf0\b0\f1\fs20 
\par 
\par 
\par \f0 The Find/Replace dialog is a generic dialog with two tabs.  One tab is used for locating text, and the other tab is used for replacing text.  The text may be searched for in a variety of places, including in open windows, as part of a project or workarea, or on the disk.  Text that is found may either be found one item at a time with navigation to each item, or a batch of locations may be sent to one of two windows dedicated to showing text that has been located.
\par 
\par When used to locate text, the Find/Replace dialog appears as follows:
\par 
\par \pard\qc\cf2\{bmc find.png\}\cf0 
\par \pard 
\par Note that some items will sometimes be closed to make the dialog smaller.
\par \f1 
\par \b\f0 Find What\b0  allows selection of the text to locate
\par 
\par \b Type\b0  allows selection of a filter to be used for file names, by default all files are searched but the scope may be narrowed to various types of files the IDE understands if necessary.
\par 
\par \b Where\b0  allows selection of what documents are in the scope of the search.  This may be one of the following
\par 
\par \pard\fi-3120\li4220\tx4260\b Current document\b0\tab any text in the window that currently has focus
\par \b Open documents\b0\tab any text in any open window
\par \b Current project\b0\tab any text in any file within the selected project
\par \b All projects\b0\tab any text in any file that exists in the workarea
\par \b CC386 include path\b0\tab any text in system include files
\par \b User specified path\b0\tab any text in a user-specified path.
\par \pard\fi1120\tx4260 
\par \pard\b Path\b0  allows selection of a path to look in for files, when Where is set to User Specified Path.  The button at the right will bring up a window that allows selection of the path, or the path may be directly typed in.  Otherwise this window doesn't appear.
\par 
\par \b How\b0  specifies how matches will be calculated.
\par 
\par \pard\fi-3080\li4180\tx4200\b Direct Match\b0\tab characters must directly match\b 
\par Regular Expressions\b0\tab\cf3\strike regular expressions\cf2\strike0\{linkID=810\}\cf0  may be used to determine the match\b 
\par Wildcard\b0\tab characters generally directly match, however \b ?\b0  may be used to match any character and \b *\b0  may be used to match any sequence of characters\b 
\par \pard\fi1160\tx4240 
\par \pard Options\b0  allow for various special case behaviors while searching.  They include
\par 
\par \pard\fi-3060\li4200\tx4180\b Match Whole Word\b0\tab when selected, any text that is matched must be the same word as \b Find What -\b0  no matches will be made on partial words
\par \b Match Case\b0\tab when selected, any text that is matched must be the same case as specified in \b  What\b0 
\par \b Recursive\b0\tab when selected in conjunction with a \b User Specified Path\b0 , will cause the search to extend to subdirectories
\par \b Search Up\b0\tab when selected, the search will be done toward the beginning of the documents instead of the end.
\par \pard\fi1140\tx4260 
\par \f1 
\par \pard\b\f0 Output\b0  selects what is going to happen when something is located.
\par 
\par \pard\fi-3060\li4200\tx4220\b Document Window\b0\tab navigate to the next location found and then stop locating text\b 
\par Find Window 1\b0\tab show the location in Find Window 1 and continue locating text\b 
\par Find Window 2\tab\b0 show the location in Find Window 2 and continue locating text\b 
\par 
\par Find Window 1 \b0 and\b  Find Window 2\b0  may be selected from tabs on the \cf3\strike Information Window\cf2\strike0\{linkID=280\}\cf0\b 
\par \pard\b0 
\par Note that for each type of location that may be specified in \b Where\b0 , options and output selections will be remembered independently.
\par \f1 
\par 
\par \f0 When used to replace text, the Find/Replace dialog appears as follows:
\par 
\par \pard\qc\cf2\f1\{bmc replace.png\}\cf0 
\par \pard 
\par \f0 Note that some items will sometimes be closed to make the dialog smaller.
\par \f1 
\par \b\f0 Find What\b0  allows selection of the text to locate
\par 
\par \b Replace With\b0  allows selection of the text to replace it with
\par 
\par \b Type\b0  allows selection of a filter to be used for file names, by default all files are searched but the scope may be narrowed to various types of files the IDE understands if necessary.
\par 
\par \b Where\b0  allows selection of what documents are in the scope of the search.  This may be one of the following
\par 
\par \pard\fi-3140\li4240\tx4260\b Current document\b0\tab any text in the window that currently has focus
\par \b Open documents\b0\tab any text in any open window
\par \b Current project\b0\tab any text in any file within the selected project
\par \b All projects\b0\tab any text in any file that exists in the workarea
\par \b CC386 include path\b0\tab any text in system include files
\par \b User specified path\b0\tab any text in a user-specified path.
\par \pard\fi1120\tx4260 
\par \pard\b Path\b0  allows selection of a path to look in for files, when Where is set to User Specified Path.  The button at the right will bring up a window that allows selection of the path, or the path may be directly typed in.  Otherwise this window doesn't appear.
\par 
\par \b How\b0  specifies how matches will be calculated.
\par \pard\fi-3100\li4240\tx4240 
\par \pard\fi-3120\li4200\tx4200\b Direct Match\b0\tab characters must directly match\b 
\par Regular Expressions\b0\tab\cf3\strike regular expressions\cf2\strike0\{linkID=810\}\cf0  may be used to determine the match\b 
\par Wildcard\b0\tab characters generally directly match, however \b ?\b0  may be used to match any character and \b *\b0  may be used to match any sequence of characters\b 
\par \pard\fi-3100\li4240\tx4240 
\par \pard Options\b0  allow for various special case behaviors while searching.  They include
\par 
\par \pard\fi-3100\li4240\tx4240\tx4260\b Match Whole Word\b0\tab when selected, any text that is matched must be the same word as \b Find What -\b0  no matches will be made on partial words
\par \b Match Case\b0\tab when selected, any text that is matched must be the same case as specified in \b Find What\b0 
\par \b Recursive\b0\tab when selected in conjunction with a \b User Specified Path\b0 , will cause the search to extend to subdirectories
\par \pard\fi1140\tx4260 
\par \f1 
\par \pard\f0 When performing a single replace, the IDE will navigate to the location that is found and show a single replacement.  When doing a Replace All, all replacements will be made by loading the files selected by 'where' into editor windows as needed, and making the changes without saving them.  The changes may then be reviewed before being accepted...
\par 
\par Note that for each type of location that may be specified in \b Where\b0 , options and output selections will be remembered independently.
\par \f1 
\par 
\par }
110
Scribble110
Project Menu




Writing



FALSE
14
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Project Menu\cf0\b0\f1\fs20 
\par 
\par \f0 The Project Menu has items related to projects.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc mproject.png\}\cf0 
\par \pard\f1 
\par \b\f0 Create New Project\b0  brings up the \cf3\strike New Project Dialog\cf2\strike0\{linkID=360\}\cf0  to allow entry of parameters for creating a new project.
\par 
\par \b Add Existing Project\b0  brings up an \cf3\strike Open File Dialog\cf2\strike0\{linkID=350\} \cf0 to allow locating an existing project to load into the current workarea.
\par 
\par \b Project Properties\b0  opens the \cf3\strike project properties dialog\cf2\strike0\{linkID=580\}\cf0  for the active project.\f1 
\par }
120
Scribble120
Tools Menu




Writing



FALSE
16
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Tools Menu
\par \cf0\b0\fs20 
\par The tools menu has options for external tools, and for configuring the IDE.  It may appear similar to what follows:
\par 
\par \pard\qc\cf2\{bmc mtools.png\}\cf0 
\par \pard\f1 
\par \f0 In this case an external tool which has been labeled '\cf1 my tool'\cf0  appears on the\b  External Tools\b0  submenu.  When selected, this will perform a specific function which has been configured through the \cf3\strike Customize\cf2\strike0\{linkID=910\}\cf0  option on the \b External Tools\b0  submenu.  Other external tools may be configured or removed through this option.
\par 
\par \b Customize\b0  (on the tools menu) brings up the \cf3\strike Toolbar Customization dialog\cf2\strike0\{linkID=860\}\cf0  which allows configuration of which toolbars will be shown, as well as the configuration of what buttons will be shown and in what order.
\par 
\par \b Build rules\b0  brings up the \cf3\strike Build Rules dialog\cf2\strike0\{linkID=880\}\cf0 , which allows customization of build rules.
\par 
\par \b Properties\b0  brings up the \cf3\strike General Properties\cf2\strike0\{linkID=550\}\cf0  page, which allows configuration of the IDE.\f1 
\par }
130
Scribble130
Edit Menu
edit menu;menu;



Writing



FALSE
26
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Edit Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Edit Menu has items which allow browsing and manipulating the text in the edit windows.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc medit.png\}\cf2 
\par \pard 
\par \b Cut, Copy\b0  and \b Paste\b0  are used to move selected text to and from the clipboard.  
\par 
\par \b Undo\b0  undoes recently made edits.  The Undo buffer is fairly deep. Note that you can undo changes even after you save a file, until the file window is closed.
\par 
\par \b Redo\b0  redoes changes that have recently been undone.
\par 
\par \b Select All\b0  selects all the text in the current edit window.
\par 
\par \b Uppercase\b0  and \b Lowercase\b0  change the selected text to either upper or lower case.
\par 
\par \pard\ri1440\b Indent\b0  moves the selected right text by one tab space.  \cf0 This may uses either spaces or tabs depending on the \cf4\strike Editor Formatting Configuration\cf3\strike0\{linkID=510\}\cf0 
\par \pard\cf2 
\par \b Unindent\b0  is similar to \b indent\b0 , except it moves the text left.\f1 
\par \cf0\f0 
\par \b Comment\b0  adds comment markers to the beginning of each line in a selected block of text, to make the text inactive code.
\par 
\par \b Uncomment\b0  removes comment markers to the beginning of each line in a selected block of text, to make the text active code again.
\par }
140
Scribble140
Window Menu
menu;window menu;



Writing



FALSE
31
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Window Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Window Menu has various items used for controlling windows in the client area of the IDE.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc mwindow.png\}
\par \pard 
\par \cf2\b Close\b0  closes the currently selected window.
\par 
\par \b Close All\b0  closes all edit windows on the client area, but not special windows that have been undocked.
\par 
\par \b New Window\b0  creates a new instance of the file in an existing window.  The windows will mirror each other, so that changes in one window are reflected in the other as well.
\par 
\par \b Save All Files\b0  saves changes to the files in all open windows.
\par 
\par \b Cascade\b0  orders all client area windows so that they are the same size and are overlapped in a staggered fashion, where the title bar of each successive window is just below and to the right of the previous window.
\par 
\par \b Tile Horizontally\b0  orders all client area windows so that they are stacked from the top of the client area to the bottom
\par 
\par \b Tile Vertically\b0  orders all client area windows so that they are stacked from the left of the client area to the right.
\par 
\par \b Arrange Icons\b0  organizes any client area windows that have been iconized.
\par 
\par The remaining elements on this menu show some of the files that are open.  The currently selected file will have a star next to it.  If there are too many open files a \b\i More Windows...\b0\i0   item will appear, which opens the \cf4\strike Select Window Dialog\cf3\strike0\{linkID=330\}.\cf0   This allows navigation to any window shown in the client area.
\par \cf2\f1 
\par \cf0\f0 Special Windows such as the Watch Window will also appear in the client area if undocked, and will show up in this list.
\par \cf2\f1 
\par \f0 Window navigation may also be performed by using the tabs at the top of the client area.
\par \f1 
\par }
150
Scribble150
Editor Context Menu
context;edit context menu;menu;



Writing



FALSE
29
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Editor Context Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Edit Context Menu is accessed by right-clicking in one of the \cf0 Editor File Windows\cf2 . It has menu items which relate to editing, browsing, and debugging.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc cmedit.png\}\cf2 
\par \pard 
\par \pard\ri1440\tx660\tx2820\cf0\b Save File\b0  saves the text in the current window to disk
\par 
\par \pard\cf2\b Cut, Copy\b0  and \b Paste\b0  are used to move selected text to and from the clipboard.  
\par 
\par \pard\ri1440\tx660\tx2820\cf0\b Toggle Bookmark\b0  toggles the bookmark setting for the current line.  See \cf4\strike Bookmarks\cf3\strike0\{linkID=400\}\cf0  for further information about bookmarks.
\par 
\par \pard\cf2\b Goto line\b0  opens the \cf4\strike Goto Dialog\cf3\strike0\{linkID=340\}\cf2 .  This allows positioning on a given line when the line number is entered.
\par \pard\ri1440\tx660\tx2820\cf0 
\par \b Browse To\b0  navigates to the definition of the word under the cursor if browse information is turned on in the \cf4\strike General Properties Hints\cf3\strike0\{linkID=530\}\cf0  page
\par 
\par \b Return To Origin\b0  opens the window the current breakpoint is associated with and navigates to the breakpoint line.
\par \b 
\par Toggle Breakpoint\b0  toggles the break point setting for the current line.  See \cf4\strike Breakpoints\cf3\strike0\{linkID=410\}\cf0  for further information about breakpoints.
\par 
\par \b Data Breakpoint\b0  opens the \cf4\strike Data Breakpoint\cf3\strike0\{linkID=890\}\cf0  dialog to allow adding a data breakpoint on the word under the cursor.
\par 
\par \b Run To Cursor\b0  starts the program being debugged if it is currently stopped, and keeps it running until either a  is hit or the flow of execution reaches the line with the cursor.
\par 
\par \b Add To Watch\b0  adds the identifier under the cursor to the \cf4\strike Watch Window\cf3\strike0\{linkID=650\} \cf0 and shows its value.  The Watch Window will be opened if it is currently closed.
\par 
\par }
160
Scribble160
View Bookmarks Menu
bookmarks;menu;view;view bookmarks menu;



Writing



FALSE
22
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 View Bookmarks Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The View Bookmarks Menu has menu items related to \cf3\strike Bookmarks\cf4\strike0\{linkID=400\}\cf2 .  It appears as follows:
\par 
\par \pard\qc\cf4\{bmc mbookmarks.png\}\cf2 
\par \pard 
\par \b Toggle Bookmark \b0 toggles the bookmark state of the line currently selected in the editor window.
\par 
\par \b Next Bookmark \b0 moves the cursor to the line of the next bookmark in the bookmarks list.  If necessary it opens a new edit window with the related file.
\par 
\par \b Previous Bookmark \b0 moves the cursor to the line of the previous bookmark in the bookmarks list.  If necessary it opens a new edit window with the related file.\f1 
\par 
\par \b\f0 Next File\b0  moves the cursor forward past all the bookmarks left in the current file, and positions to the first bookmark in the next file in the list.
\par 
\par \b Previous File\b0  moves the cursor backward past all the previous bookmarks in the current file, and positions to the first bookmark in the previous file in the list.
\par \f1 
\par \b\f0 Show Bookmarks\b0  opens the \cf3\strike Bookmark Dialog\cf4\strike0\{linkID=390\}\cf2 .  This allows the ability to browse to a specific bookmark.
\par 
\par \b Remove All Bookmarks\b0  removes all bookmarks from the bookmarks list.\f1 
\par }
170
Scribble170
View Browse Menu
menu;view;view browse menu;



Writing



FALSE
14
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 View Browse Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The View Browse Menu has menu items related to \cf3\strike Browse Information\cf4\strike0\{linkID=430\}\cf2 .  It appear as follows:
\par 
\par \pard\qc\cf4\{bmc mbrowse.png\}\cf2 
\par \pard 
\par \b Browse To\b0  opens the \cf3\strike Browse To Dialog\cf4\strike0\{linkID=380\}\cf2 .  This allows selecting a variable name to locate the definition for.
\par 
\par \b Browse\b0  uses the word under the cursor in the active editor window as a variable name to locate the definition for.
\par 
\par \b Browse Back\b0  undoes the last \b Browse \b0 or \b Browse To \b0 command.\f1 
\par }
180
Scribble180
View Menu
menu;view;view menu;



Writing



FALSE
18
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 View Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The View Menu has various items which allow for viewing windows or positioning to bookmarks and browsing.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc mview.png\}\cf2 
\par \pard 
\par \b Workarea\b0  opens or closes the \cf4\strike Workarea Window\cf3\strike0\{linkID=190\}. \cf2 When the window is displayed a check mark will appear next to the menu item.
\par 
\par \b Information Window\b0  opens or closes the \cf4\strike Information Window\cf3\strike0\{linkID=280\}\cf2 . When the window is displayed a check mark will appear next to the menu item.
\par \f1 
\par \b\f0 Bookmarks\b0  opens the \cf4\strike View Bookmarks Menu\cf3\strike0\{linkID=160\}\cf2 .
\par 
\par \b Browse\b0  opens the \cf4\strike View Browse Menu\cf3\strike0\{linkID=170\}.
\par 
\par \cf0\b Goto Line\cf2\b0  opens the \cf4\strike Goto Line\cf3\strike0\{linkID=340\}\cf2  dialog.  This allows selection of a line number to navigate to..
\par }
190
Scribble190
Workarea Window
workspace;workspace window;



Writing



FALSE
38
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1420\tx700\tx2800\cf1\b\fs32 Workarea Window\cf0\b0\f1\fs20 
\par 
\par \f0 The workarea window holds a list of projects to be created.  An example follows:
\par 
\par \pard\ri1420\qc\tx700\tx2800\cf2\{bmc wworkarea.png\}\cf0 
\par \pard\ri1420\tx700\tx2800 
\par 
\par Each project is an executable, DLL, or static library file.  Executables are programs that can be run. DLLs are like executables except they hold shared code that executables can access.  Static libraries hold code that is not itself executable but can be combined with other code to make an executable.
\par 
\par When there are multiple projects, one project will always be selected as the active project.  This is the project that will be used by default when an action such as an attempt to debug occurs.
\par 
\par Underneath each project is a list of all the source files which are required to create the project.  When a build is requested the IDE parses through the list of files to see which ones have changed, and spawns an appropriate compiler for each one.  For file types it knows about such as C language files, the IDE will automatically parse the files to make a list of dependencies.
\par 
\par New projects may be created in the workarea using the \cf3\strike Workarea Menu\cf2\strike0\{linkID=70\}\cf0  or the \cf3\strike Workarea Context Menu\cf2\strike0\{linkID=210\}\cf0 .  There are also commands to import pre-existing projects.
\par 
\par Once a project is created, a few tree items will appear underneath it.  These items hold the source files for the project.  By using the Workarea Context Menu, new files can be inserted underneath each header.  Additionally, folders may be added to organize the files.  Folders and files may be dragged around to change the heirarchy.
\par 
\par Note that the folders are not related to how data is stored in the file system.
\par 
\par When adding files to the project, the IDE will attempt to classify them and put them in one of the stock folders, which are \b Source Files\b0 , \b Include Files,\b0   and \b Resource Files\b0 .  If the associated folder has been removed it will be added again to have a place to put the file.  Once files are added they can be moved to another folder within the project.
\par 
\par Note that the \b Include Files\b0  folder is a convenience that allows one to access include files from the IDE.  The IDE will automatically calculate the dependencies of source files for purposes of the build process.  This is unrelated to the contents of the \b Include Files\b0  folder, which must be manually populated if it is desired to have any files there.
\par 
\par After a file is added to the project, double clicking on a file or selecting it and pressing enter will open that file in an editor window.
\par 
\par If a project is a library file, other projects will automatically be linked against it.
\par 
\par The workarea file stores various general settings related to the user interface, and a reference to a separate configuration file for each project.  Each project may appear in multiple workareas.  The paths stored in the configuration files are relative.  That means that projects may be moved around on the disk as long the workareas and projects remain in the same relative location to each other.
\par 
\par The arrow keys can be used to navigate through this menu.  The ENTER key opens the file the cursor is over.  CTL-INSERT and CTL-DELETE can be used to open and close sub trees.  
\par 
\par INSERT and DELETE can be used to add and remove projects and files.  INSERT opens the \cf3\strike Open File Dialog\cf2\strike0\{linkID=350\}\cf0  to allow selection of file names or new files to add.  DELETE will only display a warning if an attempt is made to remove a project; removing a file succeeds silently
\par 
\par Any project or file that is removed from the workarea still exists on the disk, and can be added again at a later time.
\par 
\par }
200
Scribble200
Client Context Menu
client context menu;context;menu;



Writing



FALSE
26
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Client Context Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Client Context Menu can be accessed by right-clicking in the client area.  It has items related to the client area such as tiling and closing/saving files.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc cmclient.png\}\cf2 
\par \pard\f1 
\par \b\f0 Close\b0  closes the currently selected window.
\par 
\par \b Close All\b0  closes all edit windows on the client area, but not special windows that have been undocked.
\par 
\par \b New Window\b0  creates a new instance of the file in the currently selected edit window.  The windows will mirror each other, so that changes in one window are reflected in the other as well.
\par 
\par \b Save All \b0 saves the contents of all open windows.
\par 
\par \b Cascade\b0  orders all client area windows so that they are the same size and are overlapped in a staggered fashion, where the title bar of each successive window is just below and to the right of the previous window.
\par 
\par \b Tile Horizontally\b0  orders all client area windows so that they are stacked from the top of the client area to the bottom
\par 
\par \b Tile Vertically\b0  orders all client area windows so that they are stacked from the left of the client area to the right.
\par 
\par \b Arrange Icons\b0  organizes any client area windows that have been iconized.
\par \f1 
\par \b\f0 More Windows...\b0  opens the \cf4\strike Select Window Dialog\cf3\strike0\{linkID=330\}\cf2  to allow a window to be selected.\f1 
\par }
210
Scribble210
Workarea Context Menu
context;menu;workspace context menu;



Writing



FALSE
27
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Workarea Context Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Workarea Context Menu has items related to the workarea.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc cmworkarea.png\}\cf2 
\par \pard 
\par 
\par \b New Project\b0  brings up the \cf4\strike New Project\cf3\strike0\{linkID=360\}\cf2  dialog for selecting the characteristics of a new project.
\par 
\par \b Existing Project\b0  brings up the \cf4\strike Open File\cf3\strike0\{linkID=350\}\cf2  dialog for importing an existing project.
\par 
\par \b Make\b0  calculates which files have changed, and recompiles those files as well as recreates any files which depend on them\b 
\par 
\par Make Active Project \b0 performs a make, but only on the active project.\b 
\par 
\par ReBuild All\b0  rebuilds all projects in the workarea by removing all output files then doing a \b make\b0 .\b 
\par 
\par Stop Build \b0 stops a build which is in progress.\b 
\par 
\par Run/Continue \b0 starts the debugger if it isn't running, at the active project.  \cf4\strike Debugging Windows\cf3\strike0\{linkID=320\}\cf2  are opened at this point.  The program will stop at the first break point it encounters, or at the start of the \i main()\i0  or \i WinMain() \i0 function if no breakpoints exist.\b 
\par 
\par Close Work Area \b0 closes the workarea.\b 
\par 
\par Properties \b0 opens the \cf4\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf2\f1 
\par }
220
Scribble220
Folder Context Menu




Writing



FALSE
20
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Folder Context Menu\cf0\b0\f1\fs20 
\par 
\par \f0 The folder context menu has items related to creating and removing folders within a project.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc cmfolder.png\}\cf0 
\par \pard 
\par \b New File\b0  opens an \cf3\strike Open File\cf2\strike0\{linkID=350\}\cf0  dialog to allow entering the name of a new file which is being created as it is added to the project.
\par 
\par \b Existing File\b0  opens an Open File dialog to allow opening an existing file to add it to the project.
\par 
\par \b New Folder\b0  creates a new folder underneath the current one.
\par 
\par \b Remove\b0  removes a folder.  Any files in the folder will be moved to the parent of the removed folder.
\par 
\par \b Rename\b0  allows renaming the folder.
\par 
\par \f1 
\par }
230
Scribble230
Project Context Menu




Writing



FALSE
34
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;\red0\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Project Context Menu\cf0\b0\f1\fs20 
\par 
\par \f0 The Project Context menu has menu items related to the project.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc cmproject.png\}\cf0 
\par \pard\f1 
\par \b\f0 New File\b0  opens an \cf3\strike Open File\cf2\strike0\{linkID=350\}\cf0  dialog to allow entering the name of a new file which is being created as it is added to the project.  The IDE will prompt for a file name when it is time to save the file.
\par 
\par \b Existing File\b0  opens an Open File dialog to allow opening an existing file to add it to the project.
\par 
\par \b New Folder\b0  creates a new folder underneath the current one.
\par 
\par \cf4\b Make\b0  calculates which files have changed, and recompiles those files as well as recreates any files which depend on them\b 
\par 
\par ReBuild \b0 rebuilds all projects in the workarea by removing all output files then doing a \b make\b0 .\b 
\par 
\par Stop Build \b0 stops a build which is in progress.\b 
\par \cf0\b0 
\par \cf4\b Run/Continue \b0 starts the debugger if it isn't running, at the active project.  \cf3\strike Debugging Windows\cf2\strike0\{linkID=320\}\cf4  are opened at this point.  The program will stop at the first break point it encounters, or at the start of the \i main()\i0  or \i WinMain() \i0 function if no breakpoints exist.\b 
\par 
\par \cf0 Run Without Debugger \b0 starts the program as a separate executable, without debugging.  If it is a console program, a separate console window will be created for it to display data in.\b 
\par \b0 
\par \b Remove\b0  removes a project from the workarea.  The project and all its files will still reside on the file system.
\par 
\par \b Rename\b0  allows renaming the project.
\par 
\par \b Build Window\b0  is an alternate way to build the project that performs the build using a make file, in a separate console window.  Errors shown in this window will not be readily accessible for locating corresponding lines in source files. 
\par 
\par \b Set As Active Project\b0  sets the current project as the active project, which selects it as the default project the debugger will use.
\par 
\par \b Properties\b0  opens the project properties window\f1 
\par }
240
Scribble240
File Context Menu




Writing



FALSE
20
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 File Context Menu\cf0\b0\f1\fs20 
\par 
\par \f0 The File Context Menu has menu items related to individual files within a project.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc cmfile.png\}\cf0 
\par \pard\f1 
\par \b\f0 Open\b0  opens an\cf3\strike  Edit Window\cf2\strike0\{linkID=50\}\cf0  with the contents of the file.
\par 
\par \b Compile File\b0  uses the appropriate tool to compile, assemble, or otherwise process the file to create an output file.
\par 
\par \b Remove\b0  removes a file from the project.  The file will still reside on the file system.
\par 
\par \b Rename\b0  allows renaming the file.
\par 
\par \b Properties\b0  opens a property window for editing build-specific properties for the file.
\par 
\par \f1 
\par }
250
Scribble250
Build Menu
build;build menu;make;menu;



Writing



FALSE
23
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Build Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Build Menu has menu items related to compiling and linking programs.  It appears as follows
\par 
\par \pard\qc\cf3\{bmc mbuild.png\}\cf2 
\par \pard 
\par \b Compile File\b0  compiles the file in the currently active editor window.\b 
\par 
\par Make\b0  calculates which files have changed, and recompiles those files as well as recreates any files which depend on them\b 
\par 
\par Make Active Project \b0 performs a make, but only on the active project.\b 
\par 
\par ReBuild All\b0  rebuilds all projects in the workarea by removing all output files then doing a \b make\b0 .\b 
\par 
\par Stop Build \b0 stops a build which is in progress.\b 
\par 
\par Generate Make File \b0 creates a \cf4\strike Generated Make File\cf3\strike0\{linkID=270\}\cf2 .
\par 
\par \b 
\par \f1 
\par }
260
Scribble260
Help Menu
help menu;menu;



Writing



FALSE
19
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Help Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Help Menu has menu items related to documentation.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc mhelp.png\}\cf2 
\par \pard 
\par \b IDE Help \b0 shows the \cf4\strike index page\cf3\strike0\{linkID=10\}\cf2  of this help file.\b 
\par 
\par Language Help\b0  shows help about the \cf4\strike C Language\cf3\strike0\{linkFile=chelp.chm\}\cf2 .
\par \b 
\par RTL Help \b0 shows help specific to the \cf4\strike C Run-time Library\cf3\strike0\{linkFile=crtl.chm\}\cf2 .
\par \b 
\par Tools Help\b0  shows help specific to the \cf4\strike Command Line Tools\cf3\strike0\{linkFile=TOOLS.chm\}\cf2 , and help related to compiler extensions used by the C compiler in this package.\b 
\par 
\par About \b0 shows the version information of the IDE.
\par \b\f1 
\par }
270
Scribble270
Generated Make File
file;generated makefile;make;



Writing



FALSE
10
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Generated Make File\cf2\b0\f1\fs20 
\par 
\par \f0 The IDE is capable of converting a workarea into a make file compatible with the IMAKE command line make utility.   This allows exporting the build steps, so that the file can be built from the command line instead of the IDE.
\par 
\par The generation is selected through the \b Generate Make File\b0  option on the \cf3\strike Build Menu\cf4\strike0\{linkID=250\}\cf2 . If the workarea has multiple project, the make file will contain scripts suitable for rebuilding all projects.
\par 
\par The paths in the make file are made relative to the location of the make file.  This means that if projects are moved to a new location, they must be kept in the same relative location to each other and to the make file.\f1 
\par }
280
IDM_INFORMATION_WINDOW
Information Window
debugging windows;information window;special windows;windows;



Writing



FALSE
22
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red255\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1440\cf1\b\fs32 Information Window\cf0\b0\f1\fs20 
\par 
\par \f0 The Information window has four tabs.   During a debug session, the debug tab is displayed and has information similar to the following when the window is docked:
\par 
\par \pard\ri1440\qc\cf2\{bmc winfo.png\}\cf0 
\par \pard\ri1440 
\par In this example, we see the IDE creating the output files for two projects.
\par 
\par The tabs will generally auto-select themselves when information is being displayed in them, or they can be manually selected at any time.
\par 
\par The \b Build\b0  tab gives information about the compile in progress.  Errors and general status can be viewed here.  Clicking on a line that shows an error or warning will open an edit window and position the file to that line.  In the build tab, errors will be in \cf3 red\cf0  and warnings will be in \cf1 blue\cf0 .  
\par 
\par The \b Debug\b0  tab gives information about the debug status, such as what DLLs and THREADs are starting or ending.  It also displays messages from the program that are sent via the OutputDebugString API function.  These messages will be in \cf1 blue\cf0 .
\par 
\par The \b Find In Files 1\b0  tab is one of the output windows for results of the \cf4\strike Find/Replace\cf2\strike0\{linkID=100\}\cf0  operation.
\par 
\par The \b Find In Files 2\b0  tab is another output window for results of the Find/Replace operation.
\par 
\par Right-Clicking on the information window pane opens the \cf4\strike Information Context Menu\cf2\strike0\{linkID=290\}\cf0 .
\par }
290
Scribble290
Information Context Menu
context;information context  menu;menu;



Writing



FALSE
17
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Information Context Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Information Context Menu is accessed by right-clicking in the \cf3\strike Information Window\cf4\strike0\{linkID=280\}\cf2 . It has menu items to aid in rebuilding the current project.  It appears as follows:
\par 
\par \pard\qc\cf4\{bmc cminfo.png\}
\par \pard 
\par \cf2\b Make\b0  calculates which files have changed, and recompiles those files as well as recreates any files which depend on them\b 
\par 
\par Make Active Project \b0 performs a make, but only on the active project.\b 
\par 
\par ReBuild All\b0  rebuilds all projects in the workarea by removing all output files then doing a \b make\b0 .\b 
\par 
\par Stop Build \b0 stops a build which is in progress.\b 
\par 
\par }
300
Scribble300
Integrated Debugger
debug;integrated debugger;



Writing



FALSE
24
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1440\cf1\b\fs32 Integrated Debugger\cf0\b0\f1\fs20 
\par 
\par \f0 The debugger is a full-featured debugger.  Providing that the executable is compiled to enable debug information, all features are available once the debugger has started.  Note that breakpoints may be set while the debugger is stopped; this allows the possibility of setting breakpoints before the program starts.
\par 
\par To start the debugger, use the \b Start Debugging\b0  item on the Debug Menu.  At this point the program will start, and run until it hits a break point.  Alternately, press F5 or F10.  If F5 is pressed, the program will start running and run until it either ends or hits a breakpoint.  If F10 is pressed the program will start running and stop at the beginning of \i main()\i0  or \i WinMain()\i0 .
\par 
\par Following is an example of an editor window when the program is stopped.
\par 
\par \pard\ri1440\qc\cf2\{bmc debugsession.png\}\cf0 
\par \pard\ri1440 
\par The dark grey lines at the left indicate which lines of the program generated executable code.  The red circle is a \cf3\strike Break point\cf2\strike0\{linkID=410\}\cf0 .  The yellow arrow indicates at what point the program has stopped.
\par 
\par Note that when the debugger stops on a function declaration, the processor stack is not set up yet.  Therefore, it is not possible to accurately determine where the function's variables are.  Local variables will appear as 'out of scope'.  Single stepping past the function prototype will cause the processor stack to be initialized.
\par 
\par There are several menu items on the \cf3\strike Editor Context Menu\cf2\strike0\{linkID=150\}\cf0  that relate to debugging.  There is also a \cf3\strike Debug Toolbar\cf2\strike0\{linkID=760\}\cf0  with some commonly used debugging functions such as starting the program, and single stepping.    An additional button on the Debug Toolbar allows the possibility of stepping out of a function.
\par 
\par Single stepping can also be performed by pressing F10 for step over, and F11 for step into.  F9 toggles breakpoints. F5 starts the program running when it is stopped.
\par 
\par The \cf3\strike Project Debug Properties\cf2\strike0\{linkID=570\}\cf0  holds settings used for setting up debugging on a project by project basis.  the \cf3\strike Hints\cf2\strike0\{linkID=530\}\cf0  properties page has settings for enabling and disabling debug hints on a global basis.
\par 
\par Only one project can be started at a time, however if some projects are DLLs which are called by the active project, the debugger will allow symbolic debugging of them.\f1 
\par }
310
Scribble310
Debug Menu
debug;debug menu;menu;



Writing



FALSE
47
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Debug Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Debug Menu has menu items related to debugging the project.  It appears as follows
\par 
\par \pard\qc\cf3\{bmc mdebug.png\}\cf2 
\par \pard\f1 
\par \b\f0 Run/Continue \b0 starts the debugger if it isn't running, at the active project.  \cf4\strike Debugging Windows\cf3\strike0\{linkID=320\}\cf2  are opened at this point.  The program will stop at the first break point it encounters, or at the start of the \i main()\i0  or \i WinMain() \i0 function if no breakpoints exist.\b 
\par 
\par Stop Debugging\b0  stops the debugger and closes Debugging Windows.\b 
\par 
\par Run \b0 starts the program if it is stopped.
\par 
\par \pard\ri1440\b Run To Cursor\b0  \cf0 runs the program.  It will stop at the next break point encountered, when the program exits, or when the program flow reaches the cursor position in the current active edit window.
\par \pard\cf2\b 
\par Stop\b0  stops the program if it is running.\b 
\par 
\par \cf0 Run Without Debugger \b0 starts the program as a separate executable, without debugging.  If it is a console program, a separate console window will be created for it to display data in.\b 
\par \cf2 
\par Add To Watch\b0  opens the \cf4\strike Add To Watch Dialog\cf3\strike0\{linkID=370\}\cf2 .  This allows an \cf4\strike expression\cf3\strike0\{linkID=800\}\cf2  to be added to the watch window.\b 
\par 
\par Hardware Breakpoints\b0  opens the \cf4\strike Hardware Breakpoints\cf3\strike0\{linkID=730\}\cf2  dialog.\b 
\par 
\par \pard\ri1440\tx660\tx2820\cf0 Data Breakpoint\b0  opens the \cf4\strike Data Breakpoint\cf3\strike0\{linkID=890\}\cf0  dialog to allow adding a data breakpoint on the word under the cursor.
\par 
\par \b Toggle Breakpoint\b0  toggles the break point setting for the current line.  See \cf4\strike Breakpoints\cf3\strike0\{linkID=410\}\cf0  for further information about breakpoints.
\par 
\par \pard\cf2\b Remove All Breakpoints\b0  removes all breakpoints from the break point list.
\par \b\f1 
\par \pard\ri1440\tx660\tx2820\cf0\f0 Return To Origin\b0  opens the window the current breakpoint is associated with and navigates to the breakpoint line.\cf2 
\par \pard 
\par The following menu exist on the \cf0\b Debug Windows\cf2\b0  submenu.  These items allow selection of various debug related windows, they are grayed out until the \cf4\strike Integrated Debugger\cf3\strike0\{linkID=300\}\cf2  is started.
\par 
\par \b Assembly\b0  opens or closes the \cf4\strike Assembly Window\cf3\strike0\{linkID=670\}\cf2 .  When the window is displayed a check mark will appear next to the menu item.
\par 
\par \b Memory\b0  opens or closes the \cf4\strike Memory Window\cf3\strike0\{linkID=680\}\cf2 .  When the window is displayed a check mark will appear next to the menu item.
\par 
\par \b Register\b0  opens or closes the \cf4\strike Register Window\cf3\strike0\{linkID=720\}\cf2 .  When the window is displayed a check mark will appear next to the menu item.
\par 
\par \b Call Stack\b0  opens or closes the \cf4\strike Call Stack Window\cf3\strike0\{linkID=700\}\cf2 .  When the window is displayed a check mark will appear next to the menu item.
\par 
\par \b Thread \b0 opens or closes the \cf4\strike Thread Window\cf3\strike0\{linkID=710\}\cf0 .  When the window is displayed a check mark will appear next to the menu item.
\par \cf3 
\par \cf0\b Watch \b0 opens or closes the \cf4\strike Watch Window\cf3\strike0\{linkID=650\}\cf2 .  When the window is displayed a check mark will appear next to the menu item.\f1 
\par \b 
\par }
320
Scribble320
Debugging Windows
debug;debugging windows;windows;



Writing



FALSE
18
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1440\cf1\b\fs32 Debugging Windows\cf0\b0\f1\fs20 
\par 
\par \f0 The IDE has several special windows used for helping to debug.  These windows are available from the \cf2\strike Debug Windows Menu\cf3\strike0\{linkID=310\}\cf0  when the debugger is running.
\par 
\par When the debugger is stopped, the IDE will close all debug windows.  However, it will remember which ones were open  and their position, and reopen them the next time the debugger is stopped
\par 
\par The debugging windows are as follows:
\par 
\par \pard\ri1440\tx660\tx4200\tab\cf2\strike Assembly Window\cf3\strike0\{linkID=670\}\cf0\tab Shows an assembly language listing 
\par \tab\cf2\strike Memory Window\cf3\strike0\{linkID=680\}\cf0\tab Shows a memory dump
\par \tab\cf2\strike Register Window\cf3\strike0\{linkID=720\}\tab\cf0 Shows registers
\par \tab\cf2\strike Call Stack Window\cf3\strike0\{linkID=700\}\cf0\tab Shows a back trace of the processor stack
\par \tab\cf2\strike Threads Window\cf3\strike0\{linkID=710\}\cf0\tab Shows a list of threads
\par \tab\cf2\strike Watch Window\cf3\strike0\{linkID=650\}\cf0\tab Shows the value of variables
\par \pard\ri1440\tx680\tx3680 
\par }
330
Scribble330
Select Window Dialog
dialog;select window dialog;



Writing



FALSE
14
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Select Window Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Select Window Dialog is accessible from the \i More Windows...\i0  item of the \cf3\strike Window Menu\cf4\strike0\{linkID=140\} \cf0 when there are enough edit windows open in the client area that the list of open windows will not fit on the Window Menu.  It is also available from the \i More Windows...\i0  item of the \cf3\strike Client Context Menu\cf4\strike0\{linkID=200\}\cf0 . It shows a list of open windows and appears similar to the following:
\par 
\par \pard\qc\cf4\{bmc morewindows.png\}
\par \pard 
\par \cf0 Double Clicking on the name of a file name will cause that window to come to the top of the view area.  
\par 
\par Special Windows such as the Watch Window will also appear in the client area if undocked, and will show up in this list.
\par 
\par \cf2\f1 
\par }
340
Scribble340
Goto Line Dialog
dialog;goto line dialog;



Writing



FALSE
12
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Goto Line Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Goto Line Dialog allows a line number to be entered.  It looks as follows:
\par 
\par \pard\qc\cf3\{bmc goto.png\}\cf2 
\par \pard 
\par When \b Ok\b0  is clicked, the currently active editor window will be positioned to the specified line.  
\par 
\par Note that it is possible to have line numbers displayed to the left of each line by selecting the \b Show Line Numbers \b0 of the \cf4\strike Basic Editor Settings\cf3\strike0\{linkID=490\}\cf2  property page.\f1 
\par }
350
Scribble350
Open File Dialog
dialog;file open;file save;new;new file;new target;new workspace;open file dialog;open target;open workspace;



Writing



FALSE
15
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Open File Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Open File Dialog is a standard windows dialog used for selecting files.  It is used to open files to edit, open workareas, create new files and workarea, locate projects to add to the workarea, and so forth.  In some cases such as opening files or adding them to a project, the open file dialog will accept a selection of multiple files.
\par 
\par The Open File Dialog appears as follows
\par 
\par \pard\qc\cf3\{bmc openfile.png\}\cf2 
\par \pard 
\par The enhanced Recent Directories combo box holds a list of directories that have been visited recently.  It may be used as a convenient method for navigating to recently used directories.
\par 
\par The title bar changes to reflect the operation being performed.  In this example, one or more text files may be opened.
\par 
\par }
360
IDH_NEW_PROJECT_DIALOG
New Project Dialog




Writing



FALSE
21
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 New Project Dialog\cf0\b0\f1\fs20 
\par 
\par \f0 The New Project Dialog allows selection of the name and location of a new project, and its type.  The project type controls what kind of output file will be created, for example an executable file or a library.
\par 
\par The New Project Dialog appears as follows:
\par 
\par \pard\qc\cf2\{bmc newproject.png\}
\par \pard 
\par \cf0\f1 
\par \f0 To create a new project, first select its type.  The following types are available:
\par 
\par \pard\fi-3100\li4180\tx4200\b Console\b0\tab a console application, the type of command line program standard C runs as.  Can also include windowing functions.\b 
\par Windows GUI\b0\tab a program with a windows GUI\b 
\par Windows DLL\b0\tab shared code and data in an executable library format of the type that Windows 32 bit requires\b 
\par Static Library\b0\tab shared code and data in a library format that can be combined with other code when creating programs.  
\par \tab This is the type of library often found in non-windows environments.
\par 
\par \pard Once the type has been chosen, select a name and location for the project.  This name does not need to have an operating system extension; an extension will be added automatically based on the file type.  This IDE will not automatically create the location, so it is necessary to select an existing directory.  Pressing the button to the right of the location will bring up a dialog that allows searching for a directory.\b\f1 
\par }
370
Scribble370
Add To Watch Dialog
add to watch dialog;dialog;



Writing



FALSE
12
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Add To Watch Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Add To Watch Dialog allows entry of the name of a variable.  It appears as follows:
\par \pard\qc 
\par \cf3\{bmc addwatch.png\}
\par 
\par \pard\cf2 Any \cf4\strike expression\cf3\strike0\{linkID=800\}\cf2  may be entered into the text field.  The combo box remembers recently selected expressions.
\par 
\par When \b Ok\b0  is pressed, the variable will appear in the \cf4\strike Watch Window\cf3\strike0\{linkID=650\}\cf2  along with its value.\f1 
\par }
380
Scribble380
Browse To Dialog
browse; browse to dialog



Writing



FALSE
10
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Browse To Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Browse To Dialog entry of a variable or function name to locate.  It appears as follows:\f1 
\par 
\par \pard\qc\cf3\{bmc browseto.\f0 png\f1\}\cf2 
\par \pard 
\par \f0 After pressing Ok, the definition of the name is located and shown in an \cf4\strike Editor Window\cf3\strike0\{linkID=50\}\cf2 .\f1 
\par }
390
Scribble390
Bookmark Dialog
bookmarks; bookmark dialog



Writing



FALSE
11
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Bookmark Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Bookmark Dialog offers a way to select from a list of bookmarks.  An example follows:
\par 
\par \pard\qc\cf3\{bmc showbookmarks.png\}\cf2 
\par \pard 
\par Double-Clicking on a bookmark opens an \cf4\strike Editor Window\cf3\strike0\{linkID=50\}\cf0  which shows the line the bookmark is on.\cf2\f1 
\par 
\par }
400
Scribble400
Bookmarks
bookmarks



Writing



FALSE
10
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1500\cf1\b\fs32 Bookmarks\cf0\b0\f1\fs20 
\par 
\par \f0 Bookmarks are used to mark positions in the text for easy traversal.  They may be turned on and off for any line in any files.  A visual indication is given to the left of the editor window as to which lines have bookmarks enabled.
\par 
\par The \cf2\strike View Bookmarks Menu\cf3\strike0\{linkID=160\}\cf0  has various items which can be used to turn bookmarks on and off, and traverse through the list of bookmarks.  The \cf2\strike Bookmark Toolbar\cf3\strike0\{linkID=780\}\cf0  has the same items.  Bookmarks may also be toggled via the \cf2\strike Editor Context Menu\cf3\strike0\{linkID=150\}\cf0  The \b Show Bookmarks \b0 item located in some of these locations brings up the \cf2\strike Bookmark Dialog\cf3\strike0\{linkID=390\}\cf0 , which can be used to select an arbitrary bookmark from the list.
\par 
\par 
\par }
410
Scribble410
Breakpoints
breakpoints



Writing



FALSE
23
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Breakpoints\cf2\b0\f1\fs20 
\par 
\par \f0 Breakpoints are set in order to bring the program to a stop.  This is useful because variables cannot be analyzed while the program is running.  Typical things to do when a break point is triggered are to look at variables in the \cf3\strike Watch Window\cf4\strike0\{linkID=650\}\cf2  or by moving the mouse over the variable name, analyze the \cf3\strike call stack\cf4\strike0\{linkID=700\}\cf2  back trace, or look at the list of running \cf3\strike threads\cf4\strike0\{linkID=710\}\cf2 .
\par 
\par Breakpoints can be set in a variety of ways, for example from the \cf3\strike Debug Toolbar\cf4\strike0\{linkID=760\}\cf2 , from the \cf3\strike Debug Menu\cf4\strike0\{linkID=310\}\cf2 ,
\par from the \cf3\strike Editor Context Menu\cf4\strike0\{linkID=150\}\cf2 , or by selecting a line in an editor window and pressing F9.
\par 
\par Breakpoints may be set while the \cf3\strike Integrated Debugger\cf4\strike0\{linkID=300\}\cf2  is stopped, but only take effect when the debugger is running.  
\par 
\par When no breakpoints are set, the program stops at the \i main()\i0  or \i Winmain()\i0  function when run, as if a breakpoint had been set there.
\par 
\par Another type of break point is implicitly set when the \b Run To\b0  item on the Debug Toolbar is used.  A break point is set at the cursor address to force the program to stop there.
\par 
\par \pard\ri1500\cf0 A breakpoint may also be set with the mouse when the debugger is running.  Simply double click in the area on the left of the \cf3\strike Editor Window\cf4\strike0\{linkID=50\}\cf0 .
\par 
\par \pard\cf2 The debugger supports \cf3\strike Hardware Breakpoints\cf4\strike0\{linkID=730\}\cf0 . This type of break point is supported by the microprocessor itself, and is used to track reads and writes to variables.  It is a simpler but faster form of tracking changes to variables, useful for small quantities such as single characters or integers.
\par 
\par Finally, the debugger supports \cf3\strike Data Breakpoints\cf4\strike0\{linkID=890\}\cf0 .  This type of breakpoint detects when a variable changes.  It can handle more complex variables than the corresponding Hardware Breakpoints, such as entire structures or arrays.  However, it uses a form of detection that has the potential to slow the program down somewhat while detection of changes is being done.
\par 
\par \cf2\f1 
\par }
420
Scribble420
Integrated Platform Help




Writing



FALSE
12
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1460\tx8520\cf1\b\fs32 Integrated Platform Help\cf0\b0\f1\fs20 
\par 
\par \f0 The IDE has four help files.  This is the first one, which is accessed by pressing F1 or by using the help buttons arranged on various dialogs.  The second one is the \cf2\strike Tools Help File\cf3\strike0\{linkFile=tools.chm\}\cf0 , which gives specific information about the command line tools the IDE utilizes to create programs.  The third one is the \cf2\strike RTL Help File\cf3\strike0\{linkFile=crtl.chm\}, \cf0 which gives specific information about run-time functions available in the C language.  The fourth one is the \cf2\strike Language Help File\cf3\strike0\{linkFile=chelp.chm\}\cf0 
\par 
\par These help files are all accessible from the \cf2\strike Help Menu\cf3\strike0\{linkID=260\}\cf0 .
\par 
\par Run Time Library help is also available for the word under the cursor by pressing SHIFT-F1.
\par 
\par 
\par }
430
Scribble430
Browse Information
browse; browse information



Writing



FALSE
16
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\b\fs32 Browse Information\cf0\b0\f1\fs20 
\par 
\par \f0 The IDE offers source code browsing.  Placing a cursor on a variable or structure name, the selecting \b Browse \b0 on the \cf2\strike View Browse Menu\cf3\strike0\{linkID=170\}\cf0  will browse to the variable or structure declaration.  Alternatively the Browse To selection will open the \cf2\strike Browse To Dialog\cf3\strike0\{linkID=380\}\cf0  and allow a name to be entered directly.
\par 
\par The browse function requires compiler support and must be enabled.  After enabling it, the project must be recompiled with the Build All option.
\par 
\par Browsing is only possible within C language files and headers.  Note that the browse information file (which is handled in the background) may become very large for large programs.
\par 
\par Browse information is also used by the \cf2\strike Jump List\cf3\strike0\{linkID=900\}.\cf0 
\par 
\par Browsing is enabled in the \cf2\strike Hints Configuration\cf3\strike0\{linkID=530\}\cf0 .
\par 
\par If a browse is requested, and browsing is not enabled, the IDE will ask if the workarea should be rebuilt with browsing enabled.  If the answer is yes, the workarea will be rebuilt as if by the \b Rebuild All \b0 selection on the \cf2\strike Build Menu\cf3\strike0\{linkID=250\}\cf0 , and then the browse will complete.
\par }
440
Scribble440
Printing
printing



Writing



FALSE
9
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\b\fs32 Printing\cf0\b0\f1\fs20 
\par 
\par \f0 The IDE provides a range of standard printing features, including multiple copy print and collating.  The contents of any open \cf2\strike Editor Window\cf3\strike0\{linkID=50\}\cf0  may be directed to the printer by using the \b Print \b0 item on the \cf2\strike File Menu\cf3\strike0\{linkID=60\}\cf0 .
\par 
\par This documentation will not attempt to cover the features of the windows printer dialog.  However, the topic \cf2\strike Printer Settings\cf3\strike0\{linkID=540\} \cf0  discusses configuration of printouts.
\par 
\par }
450
Scribble450
Toolbars
toolbars;



Writing



FALSE
23
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Toolbars\cf0\b0\f1\fs20 
\par 
\par \f0 CCIDE has six toolbars.  In general the items on the toolbar also appear somewhere in the main menu or in context menus.  The toolbars appear this way when the IDE is distributed:
\par 
\par \pard\ri1480\qc\cf2\{bmc tall.png\}\cf0 
\par \pard\ri1480 
\par The toolbars are customizable via the \cf3\strike Toolbar Customization\cf2\strike0\{linkID=860\}\cf0  dialog.  Additionally, if the ALT key is held down the mouse can be used to drag buttons around on a toolbar, or off the toolbar to remove them.
\par 
\par The toolbars may be dragged to be in a different order by holding the grip bar at the left.  They may also be dragged off the menu bar entirely and placed in the workarea.  A toolbar in the workarea looks like this:
\par 
\par \pard\ri1480\qc\cf2\{bmc tdebugclient.png\}\cf0 
\par \pard\ri1480 
\par For more information consult the following topics:
\par 
\par \pard\ri1440\tx660\tab\cf3\strike Edit Toolbar\cf2\strike0\{linkID=740\}
\par \tab\cf3\strike Navigation Toolbar\cf2\strike0\{linkID=770\}
\par \tab\cf3\strike Bookmark Toolbar\cf2\strike0\{linkID=780\}\cf0 
\par \pard\ri1460\tx680\tx3680\tab\cf3\strike Build Toolbar\cf2\strike0\{linkID=750\}
\par \cf0\tab\cf3\strike Debug Toolbar\cf2\strike0\{linkID=760\}\cf0 
\par \tab\cf3\strike Thread Toolbar\cf2\strike0\{linkID=790\}\cf0 
\par }
460
Scribble460
Toolbar Customization Dialog
dialog;toolbar customization dialog;toolbars;



Writing



FALSE
32
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Toolbar Customization Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 When a toolbar is being \cf3\strike customized\cf4\strike0\{linkID=860\}\cf2 , the Toolbar Customization Dialog appears.  This is a standard windows dialog box.  An example follows:
\par 
\par \pard\qc\cf4\{bmc tbcustom.png\}
\par \pard 
\par \cf2 This dialog configures placement of items in the toolbar, but does not affect where on the display the toolbar is placed.
\par \cf4 
\par \cf2 The left pane shows items that aren't in the toolbar.  The right pane shows items currently in the toolbar, in the order which they appear.  
\par 
\par To move an item into the toolbar, select the item in the left pane and its position in the right pane, and press the \b Add\b0  button.
\par 
\par To remove an item from the toolbar, select it in the right pane and press the \b Remove\b0  button.
\par 
\par Separators may be inserted anywhere in the toolbar.
\par 
\par \b Close\b0  closes the dialog box; changes are accepted.
\par 
\par \b Reset\b0  resets the toolbar to its default configuration.  
\par 
\par \b Move up\b0  moves an item upward in the box, which is to the left in the toolbar.
\par 
\par \b Move down\b0  moves an item downward in the box, which is to the right in the toolbar.
\par 
\par \pard\ri1480\cf0\b Help\b0  displays this text.
\par 
\par In addition to customizing the toolbar with this Dialog, pressing shift and clicking on an item in an active toolbar will allow you to drag the item elsewhere on the tool bar, or drag it off the toolbar
\par \pard\cf2 
\par \cf1\b\fs32 
\par }
470
IDH_CHOOSE_COLOR_DIALOG
Color Dialog
dialog; color dialog



Writing



FALSE
10
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Color Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Color Dialog is used to edit colors for the colorization settings.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc color.png\}\cf2 
\par \pard 
\par The dialog is shown here in its expanded state; normally when the dialog is first opened the palette at the right is not visible.  To open it use the \b Define Custom Colors\b0  button. To select a color choose from the palette on the left.  Alternatively add a color from the palette on the right to the Custom Colors box on the left, then choose the color from the custom colors box.\f1 
\par }
480
IDH_CHOOSE_FONT_DIALOG
Font Dialog
dialog; font dialog



Writing



FALSE
12
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Font Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Font Dialog is used to select a font for the editor windows.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc font.png\}\cf2 
\par \pard 
\par The \b Font\b0 , \b Style\b0  and \b Size\b0  items set font parameters.  The IDE uses bold and italicized fonts implicitly for colorization, so in general the style should be left as normal.
\par 
\par 
\par }
490
Scribble490
Basic Settings




Writing



FALSE
25
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Basic Settings\cf0\b0\f1\fs20 
\par 
\par \f0 The Basic Settings properties page allows modification of several of the most basic editor settings.  It is reached by clicking on a tree item in the \cf2\strike General Properties Dialog\cf3\strike0\{linkID=550\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc basicsettings.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \b Show Line Numbers \b0 if set to yes, line numbers will be displayed at the left of each editor window.  Otherwise they will not be displayed.  The current line number is also always shown in the status bar.\b 
\par 
\par Mouse Wheel Lines Per Tick\b0  selects the number of lines to scroll up or down if the mouse wheel is moved one click.\b 
\par 
\par Editor Font \b0 selects the font to be used in the editor.  When the arrow to the right of the font name is clicked, a \cf2\strike Font Dialog\cf3\strike0\{linkID=480\}\cf0  will appear to help locate a new font.\b 
\par \b0 
\par 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par }
500
Scribble500
Colors




Writing



FALSE
21
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Colors\cf0\b0\f1\fs20 
\par 
\par \f0 The Color Settings properties page allows modification of colors used in the editor presentation.  It is reached by clicking on a tree item in the \cf2\strike General Properties Dialog\cf3\strike0\{linkID=550\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc colorsettings.png\}
\par 
\par \pard\cf0 This page has a list of available colors, and shows a bar with each color next to its name.  Clicking on the arrow at the right of the bar brings up a \cf2\strike Color Dialog\cf3\strike0\{linkID=470\}\cf0  which may be used to edit the color.
\par 
\par Note that the auto-coloring may be turned off via the \b\strike Colorization\b0\strike0  item in the \cf2\strike Formatting Settings\cf3\strike0\{linkID=510\}\cf0  page.
\par 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par \f1 
\par }
510
Scribble510
Formatting




Writing



FALSE
30
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Formatting\cf0\b0\f1\fs20 
\par 
\par \f0 The Formatting Settings properties page allows modification of editor settings related to formatting the text.  It is reached by clicking on a tree item in the \cf2\strike General Properties Dialog\cf3\strike0\{linkID=550\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc formatting.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \b Tabs As Spaces \b0 when set to Yes, pressing the tab key will result in a number of spaces being embedded into the text, sufficient to take the cursor to the next tab stop.  When set to no, pressing the tab key will result in a single special character meaning tab being embedded in the text.  The IDE will interpret this character to mean move the cursor to the next tab stop.\b 
\par 
\par Tab Indent\b0  this is the number of single-character column positions between tab stops.\b 
\par 
\par Auto Indent \b0 when set to yes, pressing \b Enter\b0  will cause the IDE to insert enough spaces or tabs to bring the cursor to the logical position text on the line should appear at.  For most lines this means the cursor will be lined up with the position of the first character of the previous line.  However for some C language statements such as if statements there is an additional indentation to the next tab stop, to make it easier to remember to indent such lines.\b 
\par 
\par Auto Format\b0  when set to yes some formatting will be performed automatically.  This includes moving preprocessor directives to the beginning of the line, and lining up close-brackets in the same column with the corresponding open-bracket.\b 
\par 
\par Colorize\b0  when set to yes the colors on the \cf2\strike Color Settings\cf3\strike0\{linkID=500\}\cf0  page will be applied to the text in \cf2\strike Edit Windows\cf3\strike0\{linkID=50\}\cf0 , otherwise all text will be black.\b 
\par \b0 
\par 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par \f1 
\par }
520
Scribble520
Backup Settings




Writing



FALSE
28
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Backup Settings\cf0\b0\f1\fs20 
\par 
\par \f0 The Backup Settings properties page allows modification of auto-backup settings.  It is reached by clicking on a tree item in the \cf2\strike General Properties Dialog\cf3\strike0\{linkID=550\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc backupsettings.png\}
\par 
\par \pard\cf0 The backup settings, when set to yes, cause the corresponding types of files to be backed up just prior to saving a new version to disk.  To create a name for the backup file, the original file will have the string '.bak' appended to it.
\par 
\par For example, to back up a file \b MySourceFile.c\b0 , the IDE will create a new file \b MySourceFile.c.bak.\b0 
\par 
\par The following properties are available on this page:
\par 
\par \b Backup Files \b0 if set to yes, source code files will be backed up before being changed\b 
\par 
\par Backup Projects\b0  if set to yes, projects and workareas will be backed up before being changed
\par \b 
\par \b0 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par \f1 
\par }
530
Scribble530
Hints And Code Completion




Writing



FALSE
32
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Hints And Code Completion\cf0\b0\f1\fs20 
\par 
\par \f0 The Hints And Code Completion Settings properties page allows modification of editor settings related to hints and code completion.  It is reached by clicking on a tree item in the \cf2\strike General Properties Dialog\cf3\strike0\{linkID=550\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc hints.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \b Editor Hints\b0  when set to yes, editor hints will be shown while not debugging.  These involve showing the declaration of types and variables.\b 
\par 
\par Debugger Hints\b0  when set to yes, debugger hints will be shown while debugging.  These involve showing the value of variables.  At this time only simple types are displayed, structures and arrays are not displayed in the hint.\b 
\par 
\par Code Completion\b0  when set to yes, the IDE will parse source code files in the background.  When code is typed into the editor that can be interpreted as trying to access a member of a structure, the IDE will attempt to locate the structure definition and if it is available, a list of members will be displayed in a small popup window.  A member can then be selected from the popup.   Note that this only works when it is possible to parse the code; if the code is in an intermediate state (e.g. being worked on) it is possible the structure definition will not be available.\b 
\par 
\par Browse Information\b0  when set to yes, compiling the project will generate information that can aid in locating the definition of a function.  At this point, the \cf2\strike Browse Menu\cf3\strike0\{linkID=170\}\cf0  items will allow locating the definition of a variable or function name.\b 
\par 
\par Match Parenthesis\b0  when set to yes, positioning the cursor next to a bracket or parenthesis will locate a matching bracket or parenthesis and highlight both.  For example positioning the cursor on an open parenthesis will locate the matching close parenthesis and highlight both.\b 
\par 
\par Column Mark\b0  allows setting of a column position, at which a vertical line will be drawn.  The vertical line gives a visual aid as to how long the line is.  For example it could be used to try to limit how long each line is (by placing line breaks in an appropriate position).\b 
\par \b0 
\par 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par \f1 
\par }
540
Scribble540
Printer Settings




Writing



FALSE
47
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Printer Settings\cf0\b0\f1\fs20 
\par 
\par \f0 The Printer Settings properties page allows modification of printer-related settings.  It is reached by clicking on a tree item in the \cf2\strike General Properties Dialog\cf3\strike0\{linkID=550\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc printersettings.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \b Left Margin\b0  is the size of the left margin, in millimeters.\b 
\par 
\par Right Margin\b0  is the size of the right margin, in millimeters.\b 
\par 
\par Top Margin\b0  is the size of the top margin, in millimeters.\b 
\par 
\par Bottom Margin\b0  is the size of the bottom margin, in millimeters.\b 
\par 
\par 
\par Header \b0 and \b Footer \b0 give settings for formatting that should be performed on optional header and footer lines.  It isn't necessary to remember the shorthand for the options, as pressing the arrow to the right of the setting gives a menu with the options on it.  The menu appears as follows:
\par 
\par \pard\qc\cf3\{bmc mprintoptions.png\}\cf0\b 
\par \pard 
\par \b0 This menu has the following options, which are displayed on the settings line as shown in parenthesis:
\par 
\par \pard\tx1440\b\tab Time Formats\b0  include 12 hour\b (%T)\b0  and 24 hour\b (%t)\b0  time formats for the current time
\par 
\par \b\tab Date Formats\b0  include the US date\b (%D) \b0 and European date\b (%d)\b0  formats for the current date
\par 
\par \b\tab Page Number\b0  of the page currently being printed\b (%P)
\par \b0 
\par \b\tab Number of Pages\b0  in source file being printed\b (%#)\b0 
\par 
\par \b\tab File Name\b0  of source file being printed\b (%F)\b0 
\par 
\par \b\tab Left(%L)\b0 , \b Right(%R)\b0 , and \b Center(%C)\b0  allow positioning of the text in various places on the line
\par \pard 
\par 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par \f1 
\par }
550
IDH_GENERAL_PROPERTIES_DIALOG
General Properties Dialog
dialog; general properties dialog



Writing



FALSE
24
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\tx720\cf1\b\fs32 General Properties Dialog\cf0\b0\f1\fs20 
\par 
\par \pard\ri1480\f0 The General Properties Dialog is reached by selecting \b Properties \b0 on the \cf2\strike Tool Menu\cf3\strike0\{linkID=120\}.\cf0   It is used for general IDE configuration which is unrelated to projects, such as color and font configuration.
\par 
\par On the left hand side of the dialog is a tree view that lists the available pages; on the right hand side properties for a page will be shown.  Following is an example of how one of the general properties dialog pages appears.
\par 
\par \pard\ri1480\qc\cf3\{bmc hints.png\}\cf0 
\par \pard\ri1480 
\par \pard\tx720 This dialog has the following panes:
\par 
\par \pard\tx720\tx1400\b Editor Settings\b0 
\par \pard\tx1380\tx4220\tab\cf2\strike Basic Settings\cf3\strike0\{linkID=490\}\tab\cf0 Simple settings such as the font
\par \tab\cf2\strike Colors\cf3\strike0\{linkID=500\}\cf0\tab Colors to be used when highlighting source code elements
\par \tab\cf2\strike Formatting\cf3\strike0\{linkID=510\}\cf0\tab Settings related to text spacing and formatting
\par \tab\cf2\strike Backup Settings\cf3\strike0\{linkID=520\}\cf0\tab File backup parameters\cf3 
\par \pard\tx720\tx1380\tx4240 
\par \pard\tx720\tx1400\cf0\b Other Settings\b0 
\par \pard\tx1380\tx4220\tab\cf2\strike Hints and Code Completion\cf3\strike0\{linkID=530\}\tab\cf0 Settings related to automatic help given by the editor
\par \tab\cf2\strike Printer Settings\cf3\strike0\{linkID=540\}\cf0\tab Printer Settings
\par 
\par \pard\tx720 Properties specific to building programs are on the \cf2\strike Project Properties\cf3\strike0\{linkID=580\}\cf0  dialog\f1 
\par }
560
Scribble560
General Project Configuration Properties




Writing



FALSE
45
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}{\f2\fnil\fcharset0 Courier New;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 General Project Configuration Properties\cf0\b0\f1\fs20 
\par 
\par \f0 The General Project Configuration Properties page allows modification of project settings related to the type of project.  It is reached by clicking on a tree item in the \cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc general.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \b Output Path\b0  selects the place to store the intermediate compilation files, and the output executable or library file.  It is normally set to \b $(PROJECTDIR)\b0 .\b 
\par 
\par Output File\b0  selects the name of the output executable or library.  It is normally set to \b $(OUTPUTDIR)\\$(OUTPUTNAME)$(OUTPUTEXT).\b0  where \b $(OUTPUTDIR)\b0  is derived from the output path, \b $(OUTPUTNAME)\b0  is from the project name, and \b $(OUTPUTEXT)\b0  is an operating-system dependent file extension such as .EXE .DLL or .LIB that depends on the Project Type setting.\b 
\par 
\par Project Type \b0 selects the project type.  The following project types are available:
\par \b 
\par \pard\tx1440\tx4240\tab CONSOLE\tab\b0 an application designed to have a text mode interface.  This is the type of application C compilers normally create.\b 
\par \tab GUI\tab\b0 an application designed to have a windowing interface.\b 
\par \tab DLL\tab\b0 a shared library that holds code that may be executed by the operating system\b 
\par \tab Static Library\tab\b0 a shared library that holds code that may be combined with other code to make an application or DLL.\b 
\par \tab DOS\b0\tab an MSDOS application.\b 
\par \tab RAW\b0\tab an application that is raw, designed for no OS in particular.  The runtime library would not work with it very well.
\par \b 
\par \pard Library Type\b0  selects the type of C Language run-time library to use for standard functions like \f2 printf\f0 , when creating an application executable.  The following library types are available:
\par 
\par \pard\tx1420\tx4240\b\tab None\tab\b0 don't use a library.  Project types other than Static Library and Raw that don't use a library will not run under an OS.\b 
\par \tab Static Library\tab\b0 take the code from the library and put it directly in the application\b 
\par \tab LSCRTL.DLL\b0\tab use the LSCRTL.DLL that comes with this package as the library.\b 
\par \tab CRTDLL.DLL\b0\tab use the OS file CRTDLL.DLL as the library\b 
\par \tab MSVCRT.DLL\b0\tab use the OS file MSVCRT.DLL as the library\b 
\par 
\par \tab\b0 Note that in addition to the C language library, the IDE may add a static library that creates links to windowing functions.\b 
\par 
\par \pard Unicode \b0 when set to Yes, the UNICODE version of the windowing functions will be selected.  Otherwise, the ASCII version will be selected.  This does not affect functions like \f2 printf\f0  which are in the C language runtime library.\b 
\par \b0 
\par 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par \f1 
\par }
570
Scribble570
Debug Project Config Properties




Writing



FALSE
36
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}{\f2\fnil\fcharset0 Courier New;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Debug Project Config Properties\cf0\b0\f1\fs20 
\par 
\par \f0 The Debug Project Configuration Properties page allows modification of project settings related to debugging the project.  It is reached by clicking on a tree item in the \cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc debug.png\}
\par \pard 
\par \cf0 Note there is also a setting on the \cf2\strike Hints And Code Completion\cf3\strike0\{linkID=530\}\cf0  properties page which enables or disables debugger hints.  Debugger hints show the value of variables when the cursor is placed over them.\cf3 
\par \pard\qc 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \pard\tx1420\tx4240\b Create Debug Info\b0 , when set to yes creates a debug info file when the project is built\b 
\par 
\par Executable\b0  is the path to an executable to run to debug the program.  Normally it is \b $(OUTPUTFILE)\b0  which means the program generated by the project.  This is the same file specified by the \b Output File\b0  setting on the \cf2\strike General Project Configuration Page\cf3\strike0\{linkID=560\}\cf0 .  It may be necessary to change it to something else, for example when attempting to debug a DLL.\b 
\par 
\par Working Directory\b0  the path that the executed program will retrieve when it uses functions like \f2 getpwd\f0  or \f2 GetCurrentDirectory.\b 
\par \f0 
\par Command Arguments\b0  any arguments that should be passed to the executed program on the command line\b 
\par 
\par Show Return Code\b0  when set to yes, a box will appear after the program exits, showing the value that was passed to the operating system when it exited.\b 
\par 
\par Break At DLL Entry Points \b0 when set to yes, the debugger will detect the entry points of DLLs that have debug information, and stop in their initialization function such as \f2 DLLMain\f0 .\b 
\par 
\par Stop On First Chance Exception \b0 when set to no, the debugger will give the program a chance to handle any exception generated by the operating system, before declaring an error and stopping.  When set to yes, the debugger will stop immediately when an exception is raised, without giving the program a chance to handle it.
\par \b 
\par \pard\b0 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par \f1 
\par }
580
IDH_PROJECT_PROPERTIES_DIALOG
Project Properties Dialog
dialog; target properties dialog



Writing



FALSE
39
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\b\fs32 Project Properties Dialog\cf0\b0\f1\fs20 
\par 
\par \pard\ri1480\f0 The Project Properties Dialog is reached by selecting \b Properties \b0 on the \cf2\strike Workarea Context Menu\cf3\strike0\{linkID=210\},\cf0   The \cf2\strike Project Context Menu\cf3\strike0\{linkID=230\}\cf0 , or the \cf2\strike File Context Menu\cf3\strike0\{linkID=240\}\cf0 .  It can also be reached by selecting \b Properties\b0  from the \cf2\strike Project Menu\cf3\strike0\{linkID=110\}\cf0 ; this specifically opens a Properties window associated with the active project.  It is used for general IDE configuration related to building projects.
\par 
\par The Project Properties are tiered in several levels.  The same settings appear internally on each level.  
\par 
\par The top level is for the default values for properties.  These are the settings which are used by default.  
\par 
\par The default values may be overridden by selecting the properties menu item on the Workarea Context menu.  Setting values at this level sets the properties for all projects and files.
\par 
\par The Workarea properties may be overridden by selecting the properties menu item on the context menu for any project.  Setting values at this level sets the properties for the project and all files it contains, but does not affect other projects.
\par 
\par The project properties may be further overridden by selecting the properties menu item on the context menu for any file.  Setting values at this level sets the properties for the file but does not affect any other files or projects.
\par 
\par The project properties work internally by utilizing built-in macros, some of which are exposed on the property pages.  An example of this is \b PROJECTDIR\b0  which expands to the location where the project file is located.  The properties system also imports the user's environment variables, so it is possible to define entities outside the IDE and use them as parameters.
\par 
\par On the left hand side of the dialog is a tree view that lists the available pages; on the right hand side properties for a page will be shown.  Following is an example of how one of the project properties dialog pages appears.
\par 
\par \pard\ri1480\qc\cf3\{bmc pictures\\toolproperties\\linker.png\}\cf0 
\par \pard\ri1480 
\par The Project Properties Dialog has the following panes:
\par 
\par \pard\ri1460\tx1420\tx5700\b General Properties\b0 :
\par \tab\cf2\strike General Project Configuration\cf3\strike0\{linkID=560\}\cf0\tab Configuration about the project in general
\par \tab\cf2\strike Debug Project Configuration\cf3\strike0\{linkID=570\}\cf0\tab Configuration about how to debug the project
\par 
\par \b Tools Properties\b0 :
\par \tab\cf2\strike Compiler Properties\cf3\strike0\{linkID=590\}\cf0\tab Configuration for the C compiler
\par \tab\cf2\strike Assembler Properties\cf3\strike0\{linkID=600\}\cf0\tab Configuration for the Assembler
\par \tab\cf2\strike Resource Compiler Properties\cf3\strike0\{linkID=610\}\cf0\tab Configuration for the Resource Compiler
\par \tab\cf2\strike Linker Properties\cf3\strike0\{linkID=620\}\cf0\tab Configuration for the Executable File Linker
\par \tab\cf2\strike Librarian Properties\cf3\strike0\{linkID=630\}\cf0\tab Configuration for the Static Library Librarian
\par \tab\cf2\strike Custom Build Properties\cf3\strike0\{linkID=640\}\cf0\tab Configuration for things that happen after the build of a project
\par 
\par 
\par Global setup such as that for the editor is found in the \cf2\strike General Properties\cf3\strike0\{linkID=550\}\cf0  settings.
\par }
590
Scribble590
Compiler Properties




Writing



FALSE
39
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}{\f2\fnil\fcharset0 Courier New;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Compiler Properties\cf0\b0\f1\fs20 
\par 
\par \f0 The Compiler Properties page allows modification of project settings related to compiling individual C language source files with the C language compiler.  It is reached by clicking on a tree item in the \cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc compiler.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \pard\tx1420\tx4240\b Additional Preprocessor Directives\b0  is a space-delimited list of macro declarations.  This is a way to duplicate #define behavior from the command line, which allows some definitions to be kept as part of the build process instead of forcing them to reside in header files.  The macro definition has the form \f2 macroname=value\f0  or simply \f2 macroname \f0 if it is to be defined without a value.  If it is required to put spaces within the value, enclose the entire thing in quotes, for example: "\f2 macroname=my value".\b\f0 
\par 
\par Additional Include Paths\b0  is a semi-colon delimited list of paths to search for include files.  If relative paths are specified, they will be relative to the directory in which the project file is stored.\b 
\par 
\par Additional Dependencies\b0  specifies a list of additional dependencies for the file.  Normally, the IDE figures out the dependencies and it isn't necessary to use this.\b 
\par 
\par \pard Output File\b0  is the name of the output file.  It is normally set to \b $(OUTPUTDIR)\\$(OUTPUTNAME)$(OUTPUTEXT).\b0  where \b $(OUTPUTDIR)\b0  is derived from the output path, \b $(OUTPUTNAME)\b0  is the stem of the source file name, and \b $(OUTPUTEXT)\b0  is the literal '.obj'.\b 
\par \pard\tx1420\tx4240 
\par C99 \b0 is set to yes if parsing for C99 language constructs should be allowed.  Otherwise the compiler will be limited to C89 language constructs.\b 
\par 
\par Ansi\b0  is set to yes if the compiler should generate an error if one of several commonly used extensions to the ANSI standard is used.  For example, in C89 it is common for compilers to allow a comma at the end of a list of enumerations, however, the C89 ansi standard forbids it and turning on ANSI mode will disable the extension.\b 
\par 
\par Show Warnings\b0  is set to yes if the IDE should show both Warnings and Errors in the \cf2\strike Information Window\cf3\strike0\{linkID=280\}\cf0  during the build process.  Otherwise only Errors will be shown.\b 
\par 
\par Align Stack \b0 is set to yes if the compiler should generate extra code to keep the processor stack aligned to the nearest 16-byte increment when entering a new function.  This option can speed up floating point code that uses 'double' values liberally.\b 
\par 
\par Additional Switches\b0  selects additional compiler switches which aren't directly supported by the IDE.  Any switch that can be specified on the command line may be placed here.  For example, putting \b +i \b0 here will cause the compiler to generated a preprocessed output file.\b 
\par 
\par \pard\b0 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par 
\par \f1 
\par 
\par }
600
Scribble600
Assembler Properties




Writing



FALSE
29
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}{\f2\fnil\fcharset0 Courier New;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Assembler Properties\cf0\b0\f1\fs20 
\par 
\par \f0 The Assembler Properties page allows modification of project settings related to compiling individual Assembly language source files with the assembler.  It is reached by clicking on a tree item in the \cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc assembler.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par \pard\tx1420\tx4240\b 
\par Additional Preprocessor Directives\b0  is a space-delimited list of macro declarations.  This is a way to duplicate %define behavior from the command line, which allows some definitions to be kept as part of the build process instead of forcing them to reside in header files.  The macro definition has the form \f2 macroname=value\f0  or simply \f2 macroname \f0 if it is to be defined without a value.  If it is required to put spaces within the value, enclose the entire thing in quotes, for example: "\f2 macroname=my value".\b\f0 
\par 
\par Additional Include Paths\b0  is a semi-colon delimited list of paths to search for include files.  If relative paths are specified, they will be relative to the directory in which the project file is stored.\b 
\par 
\par Additional Dependencies\b0  specifies a list of additional dependencies for the file.  Normally, the IDE figures out the dependencies and it isn't necessary to use this.\b 
\par 
\par Additional Switches\b0  selects additional compiler switches which aren't directly supported by the IDE.  Any switch that can be specified on the command line may be placed here.  For example, putting \b -UMyMacro\b0  here globally undefines the macro \b MyMacro.
\par \pard\b0 
\par \b Output File\b0  is the name of the output file.  It is normally set to \b $(OUTPUTDIR)\\$(OUTPUTNAME)$(OUTPUTEXT).\b0  where \b $(OUTPUTDIR)\b0  is derived from the output path, \b $(OUTPUTNAME)\b0  is the stem of the source file name, and \b $(OUTPUTEXT)\b0  is the literal '.obj'.\b 
\par \pard\tx1420\tx4240 
\par \pard\b0 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par \f1 
\par }
610
Scribble610
Resource Compiler Properties




Writing



FALSE
30
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}{\f2\fnil\fcharset0 Courier New;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Resource Compiler Properties\cf0\b0\f1\fs20 
\par 
\par \f0 The Resource Compiler Properties page allows modification of project settings related to compiling individual resource files with the resource compiler.  It is reached by clicking on a tree item in the \cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc rc.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \pard\tx1420\tx4240\b Additional Preprocessor Directives\b0  is a space-delimited list of macro declarations.  This is a way to duplicate #define behavior from the command line, which allows some definitions to be kept as part of the build process instead of forcing them to reside in header files.  The macro definition has the form \f2 macroname=value\f0  or simply \f2 macroname \f0 if it is to be defined without a value.  If it is required to put spaces within the value, enclose the entire thing in quotes, for example: "\f2 macroname=my value".\b\f0 
\par 
\par Additional Include Paths\b0  is a semi-colon delimited list of paths to search for include files.  If relative paths are specified, they will be relative to the directory in which the project file is stored.\b 
\par 
\par Additional Dependencies\b0  specifies a list of additional dependencies for the file.  Normally, the IDE figures out the dependencies and it isn't necessary to use this.\b 
\par 
\par Additional Switches\b0  selects additional compiler switches which aren't directly supported by the IDE.  Any switch that can be specified on the command line may be placed here.\b 
\par \pard\b0 
\par \b Output File\b0  is the name of the output file.  It is normally set to \b $(OUTPUTDIR)\\$(OUTPUTNAME)$(OUTPUTEXT).\b0  where \b $(OUTPUTDIR)\b0  is derived from the output path, \b $(OUTPUTNAME)\b0  is the stem of the source file name, and \b $(OUTPUTEXT)\b0  is the literal '.res.\b 
\par \pard\tx1420\tx4240 
\par \pard\b0 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par \f1 
\par 
\par }
620
Scribble620
Linker Properties




Writing



FALSE
25
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Linker Properties\cf0\b0\f1\fs20 
\par 
\par \f0 The Linker Properties page allows modification of project settings related to creating executable files.  It is only available when the project is some executable type like CONSOLE or GUI.  It is reached by clicking on a tree item in the \cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc linker.png\}
\par 
\par \pard\cf0 The following properties are available on this page:\b 
\par \b0 
\par \pard\tx1420\tx4240\b Additional Libraries\b0  is a semi-colon delimited list of libraries to search to resolve code references.\b 
\par 
\par Additional Switches\b0  selects additional linker switches which aren't directly supported by the IDE.  Any switch that can be specified on the command line may be placed here.  For example \b FileAlign:1024 \b0 sets the PE file alignment parameter to 1024.\b 
\par \pard\b0 
\par 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par \f1 
\par 
\par 
\par }
630
Scribble630
Librarian Properties




Writing



FALSE
23
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Librarian Properties\cf0\b0\f1\fs20 
\par 
\par \f0 The Librarian Properies page allows modification of project settings related to building static libraries.  It is only available when the project type is Static Library.  It is reached by clicking on a tree item in the \cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc librarian.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \pard\tx1420\tx4240\b Page Size\b0  gives an internal alignment value.  It is a power of two greater than or equal to 16.  Lower values result in smaller libraries, but the smaller size is also a limitation so sometimes it is desirable to use larger values.  The default is 512.
\par \b 
\par \pard\b0 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par \f1 
\par 
\par 
\par }
640
Scribble640
Custom Build Properties




Writing



FALSE
28
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Custom Build Properties\cf0\b0\f1\fs20 
\par 
\par \f0 The Custom Build properties page allows defining something that can happen after a project is built.  By default it does nothing.  It is reached by clicking on a tree item in the \cf2\strike Project Properties Dialog\cf3\strike0\{linkID=580\}\cf0 .  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc custom.png\}
\par 
\par \pard\cf0 The following properties are available on this page:
\par 
\par \pard\tx1420\tx4240\b Display \b0 is a text string to display in the \cf2\strike Information Window\cf3\strike0\{linkID=280\}\cf0  when starting this build step.\b 
\par 
\par Commands \b0 is a list of commands.  Note: this version of the IDE only allows one command to be entered.\b 
\par 
\par Output Files \b0 is a list of files that will be generated by this build step.  Note: this version of the IDE only allows one output file to be entered.\b 
\par 
\par Additional Dependencies\b0  specifies a list of additional dependencies for this build step.  For this type of build step, the IDE cannot figure out any dependencies for the build step, and anything that needs to be pre-built would have to be specified here.\b 
\par 
\par \pard\b0 
\par \b Apply\b0  saves changes but leaves the dialog open
\par 
\par \b Close\b0  closes the dialog without saving changes
\par 
\par \b Accept\b0  saves changes then closes the dialog
\par 
\par \f1 
\par 
\par }
650
IDH_WATCH_WINDOW
Watch Window
debugging windows;special windows;watch window;windows;



Writing



FALSE
19
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Watch Window\cf0\b0\f1\fs20 
\par 
\par \f0 The watch window shows the value of variables, and allows them to be modified.  It appears as follows:
\par 
\par \pard\ri1480\qc\cf2\{bmc wwatch.png\}\cf0 
\par \pard\ri1480 
\par \pard\cf3 The watch window has four tabs which can independently show different groups of variables.
\par \b 
\par \pard\ri1480\cf0\b0 When a value is displayed, clicking on the right-hand side of the value will open a small text window.  Typing a new value then pressing ENTER will cause that value to be written to the executable's memory.
\par 
\par Right-Clicking in the watch window opens the \cf4\strike Watch Context Menu\cf2\strike0\{linkID=660\}\cf0 .  This allows adding or removing items to watch.
\par 
\par Right-Clicking in an edit window will display the \cf4\strike Editor Context Menu\cf2\strike0\{linkID=150\}\cf0 .  The \b Add To Watch\b0  menu item will display the word under the cursor in the current tab of the window, along with its value.
\par 
\par The window is updated automatically each time the program stops at a break point.
\par \pard\fi-2800\li2800\ri1460\tx720\tx2820\f1 
\par }
660
Scribble660
Watch Context Menu
context;menu;watch context menu;



Writing



FALSE
14
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Watch Context Menu\cf2\b0\f1\fs20 
\par 
\par \f0 The Watch Context Menu is accessed by right-clicking in the \cf3\strike Watch Window\cf4\strike0\{linkID=650\}\cf2 .  It has menu items used for working with the Watch Window.  It appears as follows:
\par 
\par \pard\qc\cf4\{bmc cmwatch.png\}\cf2 
\par \pard 
\par \b Add To Watch\b0  opens the \cf3\strike Add To Watch Dialog\cf4\strike0\{linkID=370\}\cf2  to allow a variable name to be specified.  The variable will be displayed along with its value in the current tab of the Watch Window.  If the watch window is presently closed it will be opened.
\par 
\par \b Remove From Watch\b0  removes the selected item from the Watch Window.
\par 
\par \b Clear Watch Window\b0  removes all items from the Watch Window\f1 
\par }
670
IDH_ASSEMBLY_WINDOW
Assembly Window
assembly window;debugging windows;special windows;windows;



Writing



FALSE
15
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\b\fs32 Assembly Window\cf0\b0\f1\fs20 
\par 
\par \f0 The Assembly Window shows assembly language instructions for the program.  It appears as follows when undocked:
\par 
\par \pard\ri1460\qc\cf2\{bmc wasm.png\}\cf0 
\par \pard\ri1460 
\par Generally the window will start at the position the break point is at.  However if focus is set to the window, then the 'goto' toolbar item (or CTRL-G) will bring up a dialog allowing the address to be changed.  A number can be entered here, or the name of a variable.  It is also possible to type an \cf3\strike expression\cf2\strike0\{linkID=800\}\cf0  into this box and have it evaluated.
\par 
\par In the window, disassembled instructions will be shown in blue and commentary showing the module name and line data will be shown in green.  
\par 
\par While the window has focus, the up and down arrows and page up and page down keys will allow navigation through the disassembly.  As a convenience, pressing the HOME key will show the executable code address where the program counter is currently stopped.
\par 
\par }
680
IDH_MEMORY_WINDOW
Memory Window
debugging windows;memory window;special windows;windows;



Writing



FALSE
16
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Memory Window\cf0\b0\f1\fs20 
\par 
\par \f0 The Memory Window has four tabs which can each show a block of memory.  It appears as follows when undocked:
\par 
\par \pard\ri1480\qc\cf2\{bmc wmem.png\}\cf0 
\par \pard\ri1480 
\par In each tab, the address is shown on the left, followed by a sequence of bytes, followed by a sequence of ASCII values for the byte sequence.  By default 16 bytes will be shown per line, however if the window is resized it will automatically adjust the number of bytes per line to fit.
\par 
\par At the top of the window is a small text box which is used to tell what memory should be viewed.  An address can be entered here, or the name of a variable.  It is also possible to type an \cf3\strike expression\cf2\strike0\{linkID=800\}\cf0  into this box and have it evaluated.
\par 
\par When the memory portion of the window has keyboard focus, a black cursor will appear over one of the values.  The cursor may be moved around to select a value by using the arrow and page keys.  Typing in a Hexadecimal digit will turn the cursor to blue; a second digit can be typed at this time.  While the cursor is blue, the ESCAPE key will void the edit function and return the cursor to black, or the ENTER key will accept the new value and attempt to write it to the program's memory space.  If successful the display will update with a black cursor and show the new value; if the new value cannot be written then a dialog will appear to inform you of the problem.
\par 
\par The window is automatically updated whenever the program stops at a break point.  
\par }
690
Scribble690
Memory Context Menu




Writing



FALSE
17
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Memory Context Menu\cf0\b0\f1\fs20 
\par 
\par \f0 The Memory Context Window allows selection of the size of the data displayed.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc cmmemory.png\}\cf0 
\par \pard 
\par The following data sizes are allowed:
\par 
\par BYTE - selects a byte per unit (two hexadecimal characters)
\par 
\par WORD - selects a word per unit (two bytes or four hexadecimal characters)
\par 
\par DWORD - selects a dword per unit (four bytes or eight hexadecimal characters)
\par \f1 
\par }
700
IDH_STACK_WINDOW
Call Stack Window
debugging windows;special windows;stack window;windows;



Writing



FALSE
18
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Call Stack Window\cf0\b0\f1\fs20 
\par 
\par \f0 The Call Stack Window shows a back trace of all functions that were called on the way to hitting the current break point.  This information is retrieved from a processor location known as the stack.  The window appears as follows when undocked:
\par 
\par \pard\ri1480\qc\cf2\{bmc wstack.png\}\cf0 
\par \pard\ri1480 
\par The window shows the function address and the name when debug information is available.  Sometimes debug information won't be available, such as for code that is stopped in a C language library function, or in the operating system function.  
\par 
\par The back trace will be shown for the thread that is selected in the \cf3\strike Thread Window\cf2\strike0\{linkID=710\}.\cf0   Changing which function is selected by clicking on it will be reflected in the \cf3\strike Thread Toolbar\cf2\strike0\{linkID=790\}.\cf0 
\par 
\par \pard\ri1440\tx1420\tx4240 Additionally, \cf3\strike expression\cf2\strike0\{linkID=800\}\cf0  calculations depend on the scope set up by this selection.\b 
\par \pard\ri1480\b0 
\par The back trace is shown in reverse order, with the most recent function at the top.
\par 
\par 
\par }
710
IDH_THREAD_WINDOW
Thread Window
debugging windows;special windows;thread window;windows;



Writing



FALSE
12
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\b\fs32 Thread Window\cf0\b0\f1\fs20 
\par 
\par \f0 The Thread Window shows currently active threads.  It appears as follows when undocked:
\par 
\par \pard\ri1460\qc\cf2\{bmc wthread.png\}\cf0 
\par \pard\ri1460 
\par This window shows the address each thread is at, and the name of the function it is in when available.  The thread that is at a break point is marked with a yellow arrow.
\par 
\par When a new thread is selected by clicking on it, the \cf3\strike Thread Toolbar\cf2\strike0\{linkID=790\}\cf0  will be updated, and the \cf3\strike Call Stack\cf2\strike0\{linkID=700\}\cf0  and \cf3\strike Register\cf2\strike0\{linkID=720\}\cf0  windows will reflect the contents of the selected thread.
\par }
720
IDH_REGISTER_WINDOW
Register Window
debugging windows;register window;special windows;windows;



Writing



FALSE
16
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Register Window\cf0\b0\f1\fs20 
\par 
\par \f0 The Register Window shows information about the CPU registers.  It looks as follows when undocked:
\par 
\par \pard\ri1480\qc\cf2\{bmc wregister.png\}\cf0 
\par \pard\ri1480 
\par This window has three sections.  The Arithmetic section shows information about the basic register set used for memory and mathematics operations.  The Control section shows information about registers used in CPU control, such as the program counter and stack pointer.  The Floating Point section shows information about the floating point registers.
\par 
\par Register values that changed between the last two breakpoints are shown in red; values that did not change are shown in black.  Clicking on a register value allows it to be edited.
\par 
\par \pard The register values shown will be for the thread selected in the \cf3\strike Thread Window\cf2\strike0\{linkID=710\}.
\par 
\par \cf0\f1 
\par }
730
IDH_HARDWARE_BREAKPOINTS_DIALOG
Hardware Breakpoints Dialog
breakpoints; hardware breakpoints



Writing



FALSE
18
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Hardware Breakpoints Dialog\cf2\b0\f1\fs20 
\par 
\par \f0 The Hardware Breakpoints Dialog is used to set special microprocessor breakpoints that can be triggered on data access.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc hardwarebreak.png\}\cf2 
\par \pard\f1 
\par \pard\ri1440\tx2100\f0 Hardware breakpoints are somewhat limited.  There are only four of them, and they can only be set on values that are integers or smaller.  \cf4\strike Data Breakpoints\cf3\strike0\{linkID=890\}\cf2  can often be a better approach, however, hardware breakpoints do not slow down the program like data breakpoints do.
\par 
\par Each Hardware Breakpoint has an associated name box, which holds an \cf4\strike Expression\cf3\strike0\{linkID=800\}\cf0  that determines the address the break point starts at.\cf2 
\par 
\par The breakpoints can be configured for an access \b Size\b0  in 1, 2, or 4 byte increments and set to an \b Access Type\b0  of either Write or Access.  The Write mode causes the program to stop of data is written to the specified address.  The Access mode causes the program to stop if the data is either read or written.
\par 
\par Additionally, the break point may be disabled to prevent it from triggering.
\par 
\par 
\par }
740
IDH_EDIT_TOOLBAR
Edit Toolbar
edit toolbar;toolbars;



Writing



FALSE
30
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Edit Toolbar\cf0\b0\f1\fs20 
\par 
\par \pard\ri1460\f0 The Edit toolbar has controls which are commonly used when working with source files.  It may be customized using the \cf2\strike Toolbar Customization Dialog\cf3\strike0\{linkID=860\}\cf0 .  In the undocked state, and when all buttons are in their normal position, it appears as follows:
\par \pard\ri1480 
\par \pard\ri1480\qc\cf3\{bmc tedit.png\}\cf0 
\par \pard\ri1480\f1 
\par \f0 The Edit toolbar has the following options:
\par \pard\fi-2800\li2800\ri1460\tx700\tx2800 
\par \pard\fi-4240\li4240\ri1440\tx1400\tx4260\tab\b Save\b0\tab Save the current file to disk
\par \tab\b Save All\b0\tab Save all open files to disk
\par \tab\b Print\b0\tab Open the Printer Dialog in preparation for printing the file.
\par 
\par \tab\b Cut\b0\tab Cut the selection and put it on the clipboard
\par \tab\b Copy\b0\tab Copy the selection to the clipboard
\par \tab\b Paste\b0\tab Paste the clipboard at the current cursor position
\par \tab\b Undo\b0\tab Undo the last thing done
\par \tab\b Redo\tab\b0 Redo the last thing undone
\par 
\par \tab\b To Upper Case\b0\tab the selected text in the edit window is converted to upper case
\par \tab\b To Lower Case\b0\tab the selected text in the edit window is converted to lower case
\par 
\par \tab\b Indent\tab\b0 Move the selected text to the right by one tab space 
\par \tab\tab This may uses either spaces or tabs depending on the \cf2\strike Editor Formatting\cf3\strike0\{linkID=510\} \cf0 page.
\par \tab\b Unindent\tab\b0 Move the selected text to the left by one tab space.  Once text hits the left margin it does not move backward any further.
\par 
\par \b\tab Comment\b0\tab Add comment markers to the beginning of each line in a block of selected text.\b 
\par \tab Uncomment\b0\tab Remove comment markers which are at the beginning of lines in a block of selected text.\b 
\par }
750
IDH_BUILD_TOOLBAR
Build Toolbar
build;build toolbar;make;toolbars;



Writing



FALSE
19
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\b\fs32 Build Toolbar\cf0\b0\f1\fs20 
\par 
\par \pard\ri1440\f0 The Build toolbar has controls for things that are commonly done while compiling and linking programs.  It may be customized using the \cf2\strike Toolbar Customization Dialog\cf3\strike0\{linkID=860\}\cf0 .  In the undocked state, and when all buttons are in their normal position, it appears as follows:
\par \pard\ri1460 
\par \pard\ri1460\qc\cf3\{bmc tbuild.png\}\cf0 
\par \pard\ri1460\f1 
\par \f0 The Build toolbar has the following options:
\par \pard\fi-2800\li2800\ri1440\tx700\tx2800 
\par \pard\fi-4300\li4300\ri1440\tx1440\tx4260\tab\b Compile File\b0\tab Compile the current edit window
\par \tab\b Build All\b0\tab Build out-of-date portions of all projects
\par \tab\b Build Active\b0\tab Build out-of-date portions of the active project
\par \tab\b Rebuild All\b0\tab Rebuild every part of every project
\par \f1 
\par \f0\tab\b Stop Build\tab\b0 Stop the entire build\f1 
\par \f0 
\par 
\par }
760
IDH_DEBUG_TOOLBAR
Debug Toolbar
debug;debug toolbar;toolbars;



Writing



FALSE
32
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;\red0\green0\blue0;}
\viewkind4\uc1\pard\ri1460\cf1\b\fs32 Debug Toolbar\cf0\b0\f1\fs20 
\par 
\par \f0 The Debug toolbar has controls which are useful during debugging of a program.  It may be customized using the \cf2\strike Toolbar Customization Dialog\cf3\strike0\{linkID=860\}\cf0 .  In the undocked state, and when all buttons are in their normal position, it appears as follows:
\par 
\par \pard\ri1460\qc\cf3\{bmc tdebug.png\}\cf0 
\par \pard\ri1460\f1 
\par \f0 The Debug toolbar has the following options:
\par \pard\fi-2800\li2800\ri1440\tx680\tx2800 
\par \pard\fi-4280\li4280\tx1420\tx4260\tx4260\tab\b Start/Continue debugging\b0\tab Start debugging, or run the program starting at the current breakpoint if it is already started.
\par 
\par \tab\tab If the debugger is being started, \cf2\strike Debugging Windows\cf3\strike0\{linkID=320\}\cf4  are opened.  The program will stop at a break point, or at the \i main()\i0  or \i WinMain() \i0 function if no break points are set.
\par \cf0 
\par \tab\b Stop Debugging\b0\tab Stop debugging and close the Debugging Windows.
\par 
\par \tab\b Run Without Debugger\b0\tab Run the program without debugging it.
\par 
\par \tab\b Toggle Breakpoint\b0\tab Toggle \cf2\strike Break Point\cf3\strike0\{linkID=410\}\cf0  at the current line in the currently selected edit window.
\par 
\par \tab\b Stop Program\b0\tab If the program is running, interrupt it and make it stop as if it hit a breakpoint.
\par 
\par \tab\b Run\b0\tab Run the program.  It will stop at the next break point encountered, or when the program exits.
\par \tab\b Run To Cursor\b0\tab Run the program.  It will stop at the next break point encountered, when the program exits, or when the program 
\par \tab\tab flow reaches the cursor position in the current active edit window.
\par 
\par \tab\b Step In\b0\tab Step into a subroutine and stop at the beginning.
\par \tab\b Step Over\b0\tab Step over a subroutine and break after the subroutine is done.
\par \tab\b Step Out\b0\tab Step out of the current function to the next position in the next enclosing function.
\par \f1 
\par \f0\tab\b Remove All Breakpoints\tab\b0 Delete all breakpoints to let the program run unobstructed.\f1 
\par }
770
IDH_NAV_TOOLBAR
Navigation Toolbar




Writing



FALSE
27
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Navigation Toolbar\cf0\b0\f1\fs20 
\par 
\par \pard\ri1480\f0 The Navigation toolbar has controls which are commonly used to create windows with source files, or navigate through existing windows and/or source files.  It may be customized using the \cf2\strike Toolbar Customization Dialog\cf3\strike0\{linkID=860\}\cf0 .  In the undocked state, and when all buttons are in their normal position, it appears as follows:
\par 
\par \pard\ri1480\qc\cf3\{bmc tnav.png\}\cf0 
\par \pard\ri1480\f1 
\par \f0 The Navigation toolbar has the following options\f1 
\par \pard\fi-4240\li4240\ri1440\tx1420\tx4240\f0 
\par \tab\b New\b0\tab Create a new text file
\par \tab\b Open\b0\tab Prompt for a file name to open, then open it in the edit window
\par 
\par \tab\b Back\tab\b0 Browse backwards through recently accessed files
\par \tab\b Forward\tab\b0 Browse forwards through recently accessed files
\par \tab\b Goto\b0\tab Open the \cf2\strike Goto Line Dialog\cf3\strike0\{linkID=340\}\cf0  to allow positioning to a specific line within the selected file
\par 
\par 
\par \tab\b Find\b0\tab Open the \cf2\strike Find/Replace Dialog\cf3\strike0\{linkID=100\}\cf0  to locate text in windows
\par \tab\b Find Next\b0\tab Locate the next occurrence of text
\par \tab\b Replace\b0\tab Open the Find/Replace Dialog to locate text in files or windows and replace it
\par \tab\b Find In Files\b0\tab Open the Find/Replace Dialog to locate text within a group of files
\par \pard\fi-2800\li2800\ri1460\tx700\tx2800 
\par \pard\ri1440\tx680\tx2780 Additionally the Navigation Toolbar has a dropdown combo box that can be used to search for text.  This form of search will search for simple text, in the current window.
\par 
\par \pard\fi-2800\li2800\ri1460\tx700\tx2800 
\par }
780
IDH_BOOKMARK_TOOLBAR
Bookmark Toolbar




Writing



FALSE
22
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Bookmark Toolbar\cf0\b0\f1\fs20 
\par 
\par \pard\ri1480\f0 The Bookmark toolbar has controls which are used to bookmark text in windows and navigate between bookmarks.  It may be customized using the \cf2\strike Toolbar Customization Dialog\cf3\strike0\{linkID=860\}\cf0 .  In the undocked state, and when all buttons are in their normal position, it appears as follows:
\par 
\par \pard\ri1480\qc\cf3\{bmc tbrowse.png\}\cf0 
\par \pard\ri1480\f1 
\par \f0 The Bookmark toolbar has the following options:
\par \pard\fi-2800\li2800\ri1460\tx700\tx2800 
\par \pard\fi-4520\li4520\ri1440\tx1380\tx4520\tx4560\b\tab Toggle Bookmark\b0\tab enable a bookmark at the current cursor position, or disable it if it is enabled.
\par \tab\tab a bookmark indicater will show up to the left of the line if it is enabled.\b 
\par \tab Previous Bookmark\b0\tab navigate to the previous bookmark in the list\b 
\par \tab Next Bookmark\b0\tab navigate to the next bookmark in the list\b 
\par \tab Previous File With Bookmark\b0\tab navigate to the first previous bookmark in the first previous file in the list\b 
\par \tab Next File With Bookmark\b0\tab navigate to the first next bookmark in the first next file in the list\b 
\par \tab Show Bookmarks\b0\tab show the \cf2\strike Bookmarks\cf3\strike0\{linkID=390\}\cf0  Dialog to allow selecting a bookmark\b 
\par \tab Remove All Bookmarks\b0\tab remove all bookmarks from all files
\par 
\par \b 
\par \pard\ri1440\tx1420\tx2760\tx4280 
\par }
790
IDH_THREAD_TOOLBAR
Thread Toolbar




Writing



FALSE
18
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Thread Toolbar
\par \cf0\b0\f1\fs20 
\par 
\par \pard\ri1480\f0 The Thread toolbar has two windows that allow navigation between active threads while debugging.  It may be customized using the \cf2\strike Toolbar Customization Dialog\cf3\strike0\{linkID=860\}\cf0   In the undocked state, and when all buttons are in their normal position, it appears as follows:
\par 
\par \pard\ri1480\qc\cf3\{bmc tthread.png\}\cf0 
\par \pard\ri1480\f1 
\par \f0 The Thread toolbar has the following options:
\par \pard\fi-2800\li2800\ri1460\tx700\tx2800 
\par \pard\fi-4200\li4200\ri1440\tx1400\tx4220\b\tab Threads\b0\tab a combo-box which has a list of all running threads.  Selecting one sets the active thread to it and rese breakpoints window with the call stack of that thread.
\par \tab\b Breakpoints\b0\tab a combo-box which has a list of the call stack of the selected thread.  Selecting one opens a window with a file if it is not already open, and set the cursor to the breakpoint.
\par 
\par \pard\ri1440\tx1420\tx4240 The \cf2\strike Thread\cf3\strike0\{linkID=710\}\cf0  and \cf2\strike Call Stack\cf3\strike0\{linkID=700\}\cf0  windows will be updated with the positions selected in these combo boxes.  The contents of the Call Stack and \cf2\strike Register\cf3\strike0\{linkID=720\}\cf0  windows will be updated if selections are made on this toolbar.
\par 
\par Additionally, \cf2\strike expression\cf3\strike0\{linkID=800\}\cf0  calculations depend on the scope set up by this selection.\b 
\par }
800
Scribble800
Expressions
expressions;



Writing



FALSE
16
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Expressions\cf0\b0\f1\fs20 
\par 
\par \f0 When adding a variable to the watch window with the watch window dialog, or specifying an address in the memory window edit control, it is possible to type expressions into the dialog and have them evaluated.
\par 
\par The expression evaluator knows most of the operators of the C language.  It is also possible to type-cast expressions, to tell the watch window how to display the results of expressions.
\par 
\par In general, when a variable name is typed into the dialog, that is taken to mean the address of the variable.  However when variables are used in expressions, what happens depends on the type of the variable.  If it is a basic type such as \i int\i0  or \i short,\i0  the value is fetched and used in the expression, and the result becomes a constant.  However if it is a structured type such as \i struct\i0  or \i union\i0 , or an array value, the address is still used in the expression.  Normal C language rules apply for such things; for adding constants to structure addresses results in moving further along in an array of the structures, instead of simply offsetting the base address of the structure.
\par 
\par The expression evaluator will handle typecasting of a variable name or address.
\par 
\par Sometimes an expression requires a scope, for example a variable name may appear in several different functions, each of which has called another on the way to reaching the current stop to execution flow.  In general the IDE will use the scope specified by the \cf2\strike Thread\cf3\strike0\{linkID=710\}\cf0  and \cf2\strike Call Stack\cf3\strike0\{linkID=700\}\cf0  windows, or from the current cursor position in the case of debugger hints.   It will select from a list of global variables if it cannot locate sometihng in the local scope.
\par 
\par Each expression will be completely re-evaluated each time the program stops at a break point.\f1 
\par }
810
Scribble810
Regular Expressions
regular expressions;



Writing



FALSE
59
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\fi-1640\li1640\ri1480\tx700\tx1660\cf1\b\fs32 Regular Expressions\cf0\b0\f1\fs20 
\par \pard\ri1460\tx680\tx1640 
\par \f0 Regular expressions may be used in the query boxes of the \cf2\strike Find/Replace\cf3\strike0\{linkID=100\}\cf0  dialog.  The regular expression operators are a subset of those used by the EMACS editor.  In general characters have their normal meanings, but there are a few special characters that can be used.
\par 
\par Regular expressions work by taking a list of operators with generic meanings and trying to match parts of the source text (the edit file).  If all the operators taken together in sequence match some string in the source text, the search operation is successful.
\par 
\par \pard\fi-1640\li1640\ri1480\tx700\tx1660\tab .\tab matches any single character
\par \tab +\tab matches one or more occurrences of the preceding character
\par \tab *\tab matches zero or more occurrences of the preceding character
\par \tab ?\tab matches exactly zero or one occurrences of the preceding character
\par \tab ^\tab matches beginning of line
\par \tab $\tab matches end of line
\par \tab [ and ]\tab define a list of characters which can match present character
\par \tab\tab between the brackets can be placed either a list of letters, any of which matches the current character, or the name of a character class which is matched against the current character.  The format for a character class is:
\par 
\par \tab [:classname:]
\par 
\par \pard\fi-680\li680\ri1460\tx680\tx1640\tab for example [[:alpha:]] matches any letter.  The class names follow the macros found in ctype.h:  the following class names are valid:
\par \pard\fi-1640\li1640\ri1480\tx700\tx1660 
\par \pard\fi-3360\li3360\ri1460\tx1640\tx3360\tab alpha\tab matches a letter
\par \tab upper\tab matches an upper case letter
\par \tab lower\tab matches a lower case letter
\par \tab digit\tab matches a digit
\par \tab alnum\tab matches a letter or digit
\par \tab xdigit\tab matches a hexadecimal digit
\par \tab space\tab matches any white space character, e.g. spaces,
\par \tab\tab newlines, tabs, form feeds 
\par \tab print\tab matches any printable character
\par \tab punct\tab matches punctuation characters
\par \tab graph\tab matches non-space printable characters
\par \tab cntrl\tab matches control characters
\par \tab blank\tab matches space and tab
\par 
\par \pard\fi-1640\li1640\ri1480\tx700\tx1660 
\par \tab |\tab alternator, an 'or' operator
\par \tab\\\tab escape character.  When it precedes a special character, it quotes that character.  When used with other characters it can cause special matching functions to occur
\par \tab 
\par The escape sequences are as follows:
\par 
\par \tab\\b\tab matches a word boundary
\par \tab\\B\tab matches inside a word
\par \tab\\w\tab matches word constituents
\par \tab\\W\tab matches non-word constituents
\par \tab\\<\tab matches beginning of word
\par \tab\\>\tab matches end of word
\par \tab\\`\tab matches beginning of buffer
\par \tab\\'\tab matches end of buffer
\par \tab\\( and \\)\tab define a group to be indexed in back-reference and replace operations
\par \tab\\\{ and \\\}\tab interval operators
\par \tab\tab for example: "a\{2,4\} matches from 2 to 4 'a' characters.  less than two or greater than four characters don't match
\par \tab\\digit\tab back-reference operator... references a parenthesized group
\par 
\par In the replace field most of the above sequences have no effect.  The following sequences have special meaning:
\par 
\par \tab\\digit\tab reference a parenthesized group specified in the find window
\par \tab\\\tab quote the next character\f1 
\par }
820
IDH_SAVE_FILE_DIALOG
Save File Dialog
dialog;save file dialog;



Writing



FALSE
21
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Save File Dialog\cf0\b0\f1\fs20 
\par 
\par \f0 When exiting the IDE, the IDE will check if any files in the edit windows have been modified but not saved.  If so a dialog similar to the following is displayed to the following to give the option to choose files to save:
\par 
\par \pard\ri1480\qc\cf2\{bmc savefiles.png\}\cf0 
\par \pard\ri1480 
\par Clicking on the checkbox next to a file will select or deselect it.  If a save operation is performed, only selected files will be saved.  By default all files that have changed are marked as selected, and files that have not changed are not shown in this dialog box.
\par 
\par \b Save\b0  saves the contents of the selected files to disk.  If no files are selected, save does nothing and the dialog closes.
\par 
\par \b Select All\b0  marks all files as selected.
\par 
\par \b Deselect\b0  all marks all files as unselected.
\par 
\par \b Cancel\b0  closes the dialog and aborts the exit.
\par 
\par \b Help\b0  displays this text.
\par 
\par }
830
IDH_RELOAD_FILE_DIALOG
Reload File Dialog
reload file dialog;



Writing



FALSE
22
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1480\cf1\b\fs32 Reload File Dialog\cf0\b0\f1\fs20 
\par 
\par \f0 When the IDE is brought to the foreground, it will check if the contents of any of the files in the edit windows have been changed outside the source editor, for example in another editor or as a result of running a program that writes to the file.  If any files have changed and potentially need to be reloaded, a dialog similar to the following is displayed:
\par 
\par \pard\ri1480\qc\cf2\{bmc reloadfiles.png\}\cf0 
\par \pard\ri1480 
\par Clicking on the checkbox next to a file will select or deselect it.  If a reload operation is performed, only selected files will be reloaded.  By default all files that have changed are marked as selected, and files that have not changed are not shown in this dialog box.
\par 
\par \b Reload\b0  reloads the contents of the selected files from disk.  If no files are selected, reload does nothing and the dialog closes.
\par 
\par \b Select All\b0  marks all files as selected.
\par 
\par \b Deselect\b0  all marks all files as unselected.
\par 
\par \b Cancel\b0  closes the dialog without reloading any files
\par 
\par \b Help\b0  displays this text.
\par 
\par \b 
\par }
840
Scribble840
Support For NASM
nasm support;support for nasm;



Writing



FALSE
43
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}{\f2\fnil\fcharset0 Courier New;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Support For NASM\cf2\b0\f1\fs20 
\par 
\par \f0 The IDE now has support for debugging NASM programs.  A special version of NASM is distributed with the IDE which is tailored to this product.  This version of NASM has some new features such as debug information support, and virtual segment support.  By default the IDE will install the proper version of NASM and use it; if you have other versions of NASM they won't be replaced.  However if you have installed the IDE with the option to add the IDE binaries directory to the path list, the IDE's version of NASM may be earlier on the path than the one you normally use.
\par 
\par To determine if you are running the CCIDE version of NASM, type nasm -v.  Here is an example of what will be displayed:
\par 
\par \f2\fs16\tab NASM version 0.98.38ls1 compiled on Aug 15 2004 (OMF Virtual Extensions/LADSoft)
\par 
\par \f0\fs20 Where the ls1 after the version gives the fact it is a special build from LADSoft and the build number.
\par 
\par The compile-time environment assumes certain class names for use in defining segments.  This affects both the linkage process and the debug information.  For convenience here is a list of default segment values as they would be defined in an assembly file:
\par 
\par \pard\fi-840\li840\f2\tab SECTION _TEXT BYTE CLASS=CODE USE32
\par \tab SECTION _DATA DWORD CLASS=DATA USE32
\par \tab SECTION _BSS  DWORD CLASS=BSS USE32
\par \tab SECTION _CONST  DWORD CLASS=CONST USE32
\par \tab SECTION _STRING  DWORD CLASS=STRING USE32
\par \tab GROUP DGROUP _DATA _BSS _CONST _STRING
\par 
\par \pard\f0 you may change the segment names or add new segments of the same class, but you need to stick to those class names.  if you don't have specific data to put in one of those segments, you may omit the class declaration, except that if you are creating a program without the C run time you need to define at least a few bytes of data in each of the following segments in order for windows to accept the program:
\par 
\par \f2\tab _TEXT
\par \tab _DATA
\par \tab _BSS\f0 
\par 
\par For WIN32 imported functions such as \b MessageBox\b0 , in general you want to define them as EXTERN as the IDE will include an import library that patches them to the appropriate DLL.  You would do the call like this:
\par 
\par \b\f2\tab extern MessageBoxA
\par \tab call MessageBoxA
\par 
\par \b0\f0 instead of using the bracket methodology when you define them as EXTERN.  VALX will make an import patch table which branches indirectly through the import table in the PE header.  Remember that many text-based WIN32 functions will require an 'A' or 'W' be appended to the function name to select the ASCII or UNICODE version
\par 
\par The debugger will understand integer values like 0ABH in addition to 0xAB or the decimal 171 when you are inputting numbers, for example in the register window.  It always displays them using the C language nomenclature, however.
\par 
\par The IDE uses the following command line:
\par 
\par \f2\tab nasm -s -fobj -Fls -o outfile -g filename
\par 
\par \f0 to compile the files.\f2 
\par 
\par }
850
Scribble850
Hot Keys
hot keys; keys



Writing



FALSE
102
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\ri1500\cf1\b\fs32 Hot Keys\cf0\b0\f1\fs20 
\par 
\par \f0 Many of the IDE functions may be performed with alternate key sequences.  This page gives a list of the key sequences and what they do.
\par 
\par In general, the following key sequences are always in effect:
\par 
\par \pard\ri1500\tx2760\tx2760\tx2760\b CTL-F\b0\tab Open the \cf2\strike Find/Replace Dialog\cf3\strike0\{linkID=100\}\cf0  to search for text
\par \b CTL-G\b0\tab Open the \cf2\strike Goto Line Dialog\cf3\strike0\{linkID=340\}\cf0  to position text in the current edit window to a specific line
\par \b CTL-H\b0\tab Open the Find/Replace Dialog to search for and replace text
\par \b F1\b0\tab The index page of this help file
\par \b F2\b0\tab Position to the next bookmark
\par \b SHIFT-F2\b0\tab Position to the previous bookmark
\par \b CTL-F2\tab\b0 Open the \cf2\strike Bookmark Dialog\cf3\strike0\{linkID=390\}\cf0  to select a bookmark to position to
\par \b ALT-F2\b0\tab Toggles bookmark status for the current line
\par \b F3\b0\tab Search for text again (find next)
\par \b SHIFT-F4\b0\tab Tiles the windows horizontally
\par \b CTL-F4\b0\tab Tiles the windows vertically
\par \b F5\b0\tab Start the debugger and/or run the program
\par \b SHIFT-F5\b0\tab Stop the debugger
\par \b CTL-F5\b0\tab Run the program until control flow reaches the selected line in the current editor window
\par \b ALT-F5\b0\tab Stop the program
\par \b F7\b0\tab Build the parts of all projects which are not up to date
\par \b SHIFT-F7\b0\tab Build the parts of the active project which are not up to date
\par \b CTL-F7\b0\tab Compile file in current window
\par \b F9\b0\tab Toggle break point at the selected line in the current editor window
\par \b F10\b0\tab Step over the current line
\par \b SHIFT-F10\b0\tab Run to cursor
\par \b F11\b0\tab Step into the current line
\par \b ALT-F11\b0\tab Step out of the current function
\par \b F12\b0\tab Browse to the definition of the selected word 
\par \b SHIFT-F12\b0\tab Browse back to the previous text position
\par 
\par \pard\fi-2800\li2800\ri1500\tx660\tx2800 In addition the following sequences have special meaning in the \cf2\strike Editor Window\cf3\strike0\{linkID=50\}\cf0 :
\par 
\par \pard\fi-2740\li2740\ri1500\tx2760\b CTL-C\b0\tab Copy selection to clipboard
\par \b CTL-L\b0\tab Page Break
\par \b CTL-S\b0\tab Save the file in the current edit window
\par \b CTL-V\b0\tab Paste clipboard to cursor position.  This overwrites the selection if there is one
\par \b CTL-T\b0\tab Place the line the cursor is at in the center of the window (vertical center)
\par \b CTL-X\b0\tab Copy the selection to the clipboard, and cut it from the text
\par \b CTL-Z\b0\tab Undo the last operation
\par \b SHIFT-F1\tab\b0 Show run time library help for the word under the cursor.
\par \b CTL-F7\b0\tab Compile the file in the source window
\par \b BACKSPACE\b0\tab Delete the character preceding the cursor
\par \b DELETE\b0\tab Delete the character at the cursor
\par \b END\b0\tab Move the cursor to the end of the line
\par \b CTL-END\b0\tab Move the cursor to the end of the file
\par \b HOME\b0\tab Move the cursor to the beginning of the line
\par \b CTL-HOME\b0\tab Move the cursor to the beginning of the file
\par \b INSERT\b0\tab toggles the insert mode between INSERT and OVERWRITE
\par \b PAGE DOWN\b0\tab Move the cursor down one page
\par \b PAGE UP\b0\tab Move the cursor up one page
\par \b TAB\b0\tab Insert a tab character, or indent text if it is selected.  Depending on the \cf2\strike Editor Formatting Configuration\cf3\strike0\{linkID=510\}\cf0 , the tab may be replaced by enough spaces to reach the next tab position.
\par \b SHIFT-TAB\b0\tab Delete a tab or spaces to the left of the cursor, or unindent if text is selected\f1 
\par \b\f0 DOWN ARROW\b0\tab Navigate downward one line
\par \b UP ARROW\b0\tab Navigate upward one line
\par \b LEFT ARROW\b0\tab Navigate left one character
\par \b RIGHT ARROW\b0\tab Navigate right one character
\par \b CTL-RIGHT ARROW\b0  \tab Navigate right one word
\par \b CTL-LEFT ARROW \b0\tab Navigate left one word
\par 
\par \pard\fi-2800\li2800\ri1500\tx660\tx2800 The following keys have special meaning in the \cf2\strike Workarea Window\cf3\strike0\{linkID=190\}\cf0 :
\par 
\par \pard\fi-2780\li2780\ri1500\tx2780\b DOWN ARROW\b0\tab Navigate downward one line
\par \b UP ARROW\b0\tab Navigate upward one line
\par \b LEFT ARROW\b0\tab Navigate left one character
\par \b RIGHT ARROW\b0\tab Navigate right one character
\par \b INSERT\b0\tab If the selection is on a project, open the \cf2\strike Open File Dialog\cf3\strike0\{linkID=350\}\cf0  and prompt for a new project to insert prior to this one.  Otherwise open the Open File Dialog and prompt for new files to insert in the selected project.
\par \b CTL-INSERT  \b0\tab Expand a node
\par \b DELETE\b0\tab If the selection is on a project, a prompt will appear asking whether to remove it from the workarea.  Accepting it will remove the project.  If the selection is on a file, the file will be removed from the workarea.
\par \b CTL-DELETE \b0\tab Collapse a node
\par \b ENTER\b0  \tab open the file corresponding to the node
\par 
\par \pard\fi-2800\li2800\ri1500\tx660\tx2800 The following keys have special meaning in the \cf2\strike Assembly Window\cf3\strike0\{linkID=670\}\cf0 :
\par 
\par \pard\fi-2780\li2780\ri1500\tx2760\b CTL-G\b0\tab Prompt for an address to position to
\par \b HOME\tab\b0 Navigate to the position the program is currently stopped at
\par \b DOWN ARROW\b0\tab Navigate downward one line
\par \b UP ARROW\b0\tab Navigate upward one line
\par \b LEFT ARROW\b0\tab Navigate left one character
\par \b RIGHT ARROW\b0\tab Navigate right one character
\par \b PAGE-UP\b0\tab Navigate upward one page
\par \b PAGE-DOWN\b0\tab Navigate downward one page
\par 
\par \pard\fi-2800\li2800\ri1500\tx660\tx2800 The following keys have special meaning in the \cf2\strike Memory Window\cf3\strike0\{linkID=680\}\cf0 :
\par 
\par \pard\fi-2780\li2780\ri1500\tx2760\tx2780\b DOWN ARROW\b0\tab Navigate downward one line
\par \b UP ARROW\b0\tab Navigate upward one line
\par \b LEFT ARROW\b0\tab Navigate left one character
\par \b RIGHT ARROW\b0\tab Navigate right one character
\par \b PAGE-UP\b0\tab Navigate upward one page
\par \b PAGE-DOWN\b0\tab Navigate downward one page
\par \tab These keys allow navigation through the memory
\par \b 0-9, A-F\b0\tab These keys let you enter a new value.  The characters will turn blue until ENTER is pressed to accept the change
\par \b ENTER\tab\b0 Accept a new value and program it into memory
\par \b ESCAPE\b0\tab Restore the original value for a change that has not been accepted
\par 
\par Most of the debug windows allow use of \b CTL-C\b0  to copy text from the window onto the clipboard.
\par \pard\cf1\b\fs32 
\par }
860
IDH_TOOLBAR_SELECT_DIALOG
Customize Tools




Writing



FALSE
13
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Customize Tools\cf0\b0\f1\fs20 
\par \cf2\f0 
\par The Customize Tools Dialog is accessible from the \cf3\strike Tools Menu\cf4\strike0\{linkID=120\}.  \cf0 It \cf2 shows a list of toolbars.  The dialog appears as follows:
\par 
\par \pard\qc\cf4\{bmc customizetoolbar.png\}\cf2 
\par 
\par \pard Here each of the six toolbars is listed, with a checkbox to indicate whether the toolbar is selected or deselected.  Selected toolbars appear on the display; deselected toolbars do not.
\par 
\par \cf0 Pressing the Customize button will open the \cf3\strike Toolbar Customization\cf4\strike0\{linkID=460\}\cf0  dialog, which allows rearranging the buttons on the toolbar.
\par \f1 
\par }
880
IDH_BUILD_RULE_DIALOG
Build Rules




Writing



FALSE
9
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Build Rules\cf0\b0\f1\fs20 
\par 
\par \f0 The Build Rules dialog allows disabling internal build rules.  In a future version of the IDE it will also allow creating and editing new build rules.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc buildrules.png\}\cf0 
\par \pard\f1 
\par }
890
IDH_DATA_BREAKPOINTS_DIALOG
Data Breakpoint




Writing



FALSE
17
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Data Breakpoint\cf0\b0\f1\fs20 
\par 
\par \cf2\f0 The Data Breakpoints Dialog is used to set special microprocessor breakpoints that can be triggered on data access.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc databreakpoint.png\}\cf2 
\par \pard\f1 
\par \pard\ri1440\tx2100\f0 Data Breakpoints are general purpose breakpoints.  They can be set to trigger when any variable is accessed, including a complete structure or an array.  However, a data breakpoint slows down the program by a small amount.  In some cases where the limits of \cf4\strike Hardware Breakpoints\cf3\strike0\{linkID=730\}\cf2  can be tolerated, they are a better choice.
\par 
\par Each Data Breakpoint is simply an \cf4\strike Expression\cf3\strike0\{linkID=800\}\cf0  that determines the address the break point starts at.\cf2 
\par 
\par Data Breakpoints can be added and removed through the dialog.  Additionally, a checkbox to the left of the breakpoint allows the possibility of disabling breakpoint while keeping it in the list.
\par 
\par 
\par \pard\cf0\f1 
\par }
900
Scribble900
Jump List




Writing



FALSE
15
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Jump List\cf0\b0\f1\fs20 
\par 
\par \f0 The Jump List is a pair of combo boxes that appear just below the source file tabs, and above the edit windows which hold source files.  It allows selection of a type of global variable in one combo box;  when the type of the variable is selected the other combo box will be populated with variables of that type.  Selecting a variable from the second combo box causes the IDE to position the cursor to that function.
\par 
\par The possible types are:
\par 
\par \pard\tx1380\b\tab Global Functions
\par \tab Global Variables
\par \tab Global Types
\par \tab Preprocessor Macros
\par 
\par \pard\b0 Note that the Jump List depends on the program having previously been compiled with\cf2\strike  Browse Information\cf3\strike0\{linkID=430\}\cf0  enabled on the \cf2\strike Hints and Code Completion\cf3\strike0\{linkID=530\}\cf0  properties page.\f1 
\par }
910
IDH_CUSTOM_TOOLS_DIALOG
Custom Tools




Writing



FALSE
18
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}}
{\colortbl ;\red0\green0\blue255;\red128\green0\blue0;\red0\green128\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Custom Tools\cf0\b0\f1\fs20 
\par 
\par \f0 The Custom Tools dialog allows configuration of programs that can be run directly from the IDE.  It appears as follows:
\par 
\par \pard\qc\cf2\{bmc externaltools.png\}\cf0 
\par \pard 
\par This dialog shows a list of custom tools that have previously been defined.  Each tool has a checkmark, which indicates whether the tool should show up in the \cf3\strike Tools -> External Tools menu\cf2\strike0\{linkID=120\}.  \cf0 When a tool appears in the Tools menu, clicking on it will execute the program associated with the tool
\par 
\par The items will show up in the Tools menu in the same order they are displayed in this dialog, so there \b Move Up\b0  and \b Move Down \b0 options that move the selected item around.
\par 
\par \b Add\b0  creates a new item and opens the \cf3\strike Custom Tools Editor Dialog\cf2\strike0\{linkID=920\}\cf0  to set it up.
\par 
\par \b Edit\b0  opens the Custom Tools Editor dialog to edit the selected item.
\par 
\par \b Remove\b0  removes the selected item from the list of tools.\f1 
\par }
920
IDH_CUSTOM_TOOLS_EDITOR
Custom Tools Editor




Writing



FALSE
19
{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Arial;}{\f1\fnil Arial;}{\f2\fnil\fcharset0 Courier New;}}
{\colortbl ;\red0\green0\blue255;\red0\green128\blue0;\red128\green0\blue0;}
\viewkind4\uc1\pard\cf1\b\fs32 Custom Tools Editor\cf0\b0\f1\fs20 
\par 
\par \f0 The Custom Tools Editor allows modifying the name of the executable a \cf2\strike Custom Tool\cf3\strike0\{linkID=910\}\cf0  menu item will run, as well as parameters.  It appears as follows:
\par 
\par \pard\qc\cf3\{bmc externaltoolscustomize.png\}\cf0 
\par \pard\f1 
\par \f0 When a custom tool is selected from the \cf2\strike Tools Menu\cf3\strike0\{linkID=120\},\cf0  the IDE will use the information specified in this dialog to run the tool.
\par 
\par Several text boxes appear on the dialog:
\par 
\par \pard\tx1380\tx4220\b\tab Menu Name\b0\tab name of the item as it will appear on the menu\b 
\par \tab Command\b0\tab name of the program to run.  The PATH environment variable will be searched for the program.\b 
\par \tab Arguments\b0\tab any command-line arguments to be provided when the program runs.\b 
\par \tab Working Directory\b0\tab the directory the program will see if it calls a function like \f2 getpwd\f0  or \f2 GetCurrentDirectory\b\f0 
\par 
\par \pard\tx1400\b0 Additionally, the \b Open Command\b0  Window checkbox indicates whether the IDE should open an operating system command prompt to run the program in.\f1 
\par }
0
0
0
136
1 General Information
2 Welcome=Scribble10
2 Getting Started=Scribble20
2 Using With NASM=Scribble840
2 GNU CopyLeft Statement=Scribble30
1 Bookmarks
2 Bookmarks=Scribble400
2 Bookmark Menu=Scribble160
2 Bookmark Toolbar=IDH_BOOKMARK_TOOLBAR
2 Bookmark Dialog=Scribble390
1 Breakpoints
2 Breakpoints=Scribble410
2 Hardware Breakpoints Dialog=IDH_HARDWARE_BREAKPOINTS_DIALOG
2 Data Breakpoint Dialog=IDH_DATA_BREAKPOINTS_DIALOG
1 Browse Information
2 Browse information=Scribble430
2 Browse Menu=Scribble170
2 Jump List=Scribble900
2 General Configuration=IDH_GENERAL_CONFIGURATION
1 Dialogs
2 Add To Watch Dialog=Scribble370
2 Bookmark Dialog=Scribble390
2 Browse To Dialog=Scribble380
2 Color Dialog=IDH_CHOOSE_COLOR_DIALOG
2 Custom Tools Dialog=IDH_TOOLBAR_SELECT_DIALOG
2 Custom Tools Editor Dialog=IDH_CUSTOM_TOOLS_EDITOR
2 Customize Tools Dialog=IDH_TOOLBAR_SELECT_DIALOG
2 Data Breakpoint Dialog=IDH_DATA_BREAKPOINTS_DIALOG
2 Find Dialog=Scribble310
2 Find in Files Dialog=Scribble280
2 Font Dialog=IDH_CHOOSE_FONT_DIALOG
2 General Properties Dialog=IDH_GENERAL_PROPERTIES_DIALOG
2 Goto Line Dialog=Scribble340
2 New Project Dialog=IDH_NEW_PROJECT_DIALOG
2 Open File Dialog=Scribble350
2 Project Properties Dialog=IDH_PROJECT_PROPERTIES_DIALOG
2 Reload File Dialog=IDH_RELOAD_FILE_DIALOG
2 Replace Dialog=Scribble320
2 Save File Dialog=IDH_SAVE_FILE_DIALOG
2 Select Window Dialog=Scribble330
2 Project Properties Dialog=IDH_PROJECT_PROPERTIES_DIALOG
2 Toolbar Customization Dialog=Scribble460
1 External Tools
2 Custom Tools=IDH_TOOLBAR_SELECT_DIALOG
2 Custom Tools Editor=IDH_CUSTOM_TOOLS_EDITOR
1 Editor
2 Editor Window=Scribble50
2 Editor Configuration=IDH_GENERAL_PROPERTIES_DIALOG
3 Basic Settings=Scribble490
3 Colors=Scribble500
3 Formatting=Scribble510
3 Backup Settings=Scribble520
3 Hints And Code Completion=Scribble530
3 Printer Settings=Scribble540
2 Edit Toolbar=IDH_EDIT_TOOLBAR
2 Edit Menu=Scribble130
2 Editor Context Menu=Scribble150
1 Help
2 Integrated Platform Help=Scribble420
2 Install Configuration=IDH_INSTALL_CONFIGURATION
2 Help Menu=Scribble260
1 Information Windows
2 Information Window=IDM_INFORMATION_WINDOW
2 Information Context Menu=Scribble290
1 Integrated Debugger
2 Integrated Debugger=Scribble300
2 Debug Target Properties=IDH_DEBUG_CONFIGURATION
2 Debug Toolbar=IDH_DEBUG_TOOLBAR
2 Debug Menu=Scribble310
2 Expressions=Scribble800
2 Debugging Windows=Scribble320
3 Hardware Breakpoints=IDH_HARDWARE_BREAKPOINTS_DIALOG
3 Data Breakpoints=IDH_DATA_BREAKPOINTS_DIALOG
3 Assembly Window=IDH_ASSEMBLY_WINDOW
3 Memory Window=IDH_MEMORY_WINDOW
3 Register Window=IDH_REGISTER_WINDOW
3 Call Stack Window=IDH_STACK_WINDOW
3 Threads Window=IDH_THREAD_WINDOW
3 Watch Window=IDH_WATCH_WINDOW
1 Menus
2 Menus=Scribble40
2 Main Menu
3 File Menu=Scribble60
4 Workarea Menu=Scribble70
4 File SubMenu=Scribble80
3 Edit Menu=Scribble130
3 View Menu=Scribble180
4 View Browse Menu=Scribble170
4 View Bookmarks Menu=Scribble160
3 Search Menu=Scribble90
3 Project Menu=Scribble110
3 Build Menu=Scribble250
3 Debug Menu=Scribble310
3 Window Menu=Scribble140
3 Help Menu=Scribble260
2 Context Menus
3 Client Context Menu=Scribble200
3 Editor Context Menu=Scribble150
3 Information Context Menu=Scribble290
3 Memory Context Menu=Scribble690
3 Watch Context Menu=Scribble660
3 Workarea Context Menu=Scribble210
3 Folder Context Menu=Scribble220
3 Project Context Menu=Scribble230
3 File Context Menu=Scribble240
1 Workarea Window
2 Workarea Window=Scribble190
2 Build Toolbar=IDH_BUILD_TOOLBAR
2 Build Menu=Scribble250
2 Workarea Context Menu=Scribble210
2 New Project Dialog=IDH_NEW_PROJECT_DIALOG
2 Generated Make File=Scribble270
2 Project Properties Dialog=IDH_PROJECT_PROPERTIES_DIALOG
3 Debug Project Config Properties=Scribble570
3 General Project Configuration Properties=Scribble560
3 Compiler Properties=Scribble590
3 Assembler Properties=Scribble600
3 Resource Compiler Properties=Scribble610
3 Linker Properties=Scribble620
3 Librarian Properties=Scribble630
3 Custom Build Properties=Scribble640
1 Support Functions
2 Find and Replace=IDH_FIND_REPLACE_DIALOG
2 Hot Keys=Scribble850
2 Printing=Scribble440
2 Regular Expressions=Scribble810
1 Toolbars
2 Toolbars=Scribble450
2 Toolbar Customization=IDH_TOOLBAR_SELECT_DIALOG
3 Toolbar Editor=Scribble870
2 Edit Toolbar=IDH_EDIT_TOOLBAR
2 Navigation Toolbar=IDH_NAV_TOOLBAR
2 Build Toolbar=IDH_BUILD_TOOLBAR
2 Debug Toolbar=IDH_DEBUG_TOOLBAR
2 Bookmark Toolbar=IDH_BOOKMARK_TOOLBAR
2 Thread Toolbar=IDH_THREAD_TOOLBAR
0
