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
/*                                 DEFINES.H                               */

#define Now                            get_time()

#define MAX_ELEMENT_SIZE               4100
#define MAX_OBJECT_FILE_READ_SIZE      4096
#define MAX_PUBLICS_IN_LIBRARY         16384
#define TEMP_STRING_LENGTH             512
/* The following string must be one more than TEMP_STRING_LENGTH */
#define TEMP_STRING_SIZE               (TEMP_STRING_LENGTH + 1)

#define AlignmentGap(exp, bound)       (-(exp And bound) And bound)
#define CanonicFrame(exp)              ((bit_16) (exp >> 4L))
#define Frame(exp)                     ((bit_32) ((*(*exp).segment).address & \
                                                  0xFFFFFFF0L))
#define Target(exp)                    ((bit_32) ((*exp).address-Frame(exp)))
#define TokenIs(exp)                   (compare_string(token,exp) == 0)
#define TokenIsNot(exp)                (compare_string(token,exp) != 0)
#define TokenStartsWith(exp)           (compare_short_string(token,exp) == 0)

#define PE_BASE_HEADER_SIZE     0x18
#define PE_OPTIONAL_HEADER_SIZE 0xe0
#define PE_OBJECTENTRY_SIZE     0x28
#define PE_HEADBUF_SIZE         (PE_BASE_HEADER_SIZE+PE_OPTIONAL_HEADER_SIZE)
#define PE_IMPORTDIRENTRY_SIZE  0x14
#define PE_NUM_VAS              0x10
#define PE_EXPORTHEADER_SIZE    0x28
#define PE_RESENTRY_SIZE        0x08
#define PE_RESDIR_SIZE          0x10
#define PE_RESDATAENTRY_SIZE    0x10
#define PE_SYMBOL_SIZE          0x12
#define PE_RELOC_SIZE           0x0a

#define PE_ORDINAL_FLAG    0x80000000
#define PE_INTEL386        0x014c
#define PE_MAGICNUM        0x010b
#define PE_FILE_EXECUTABLE 0x0002
#define PE_FILE_32BIT      0x0100
#define PE_FILE_LIBRARY    0x2000
#define PE_FILE_REVERSE_BITS_HIGH 0x8000
#define PE_FILE_REVERSE_BITS_LOW  0x80
#define PE_FILE_LOCAL_SYMBOLS_STRIPPED 8
#define PE_FILE_LINE_NUMBERS_STRIPPED 4

#define PE_REL_LOW16 0x2000
#define PE_REL_OFS32 0x3000

#define PE_SUBSYS_NATIVE  1
#define PE_SUBSYS_WINDOWS 2
#define PE_SUBSYS_CONSOLE 3
#define PE_SUBSYS_POSIX   7

#define WINF_UNDEFINED   0x00000000
#define WINF_CODE        0x00000020
#define WINF_INITDATA    0x00000040
#define WINF_UNINITDATA  0x00000080
#define WINF_DISCARDABLE 0x02000000
#define WINF_NOPAGE      0x08000000
#define WINF_SHARED      0x10000000
#define WINF_EXECUTE     0x20000000
#define WINF_READABLE    0x40000000
#define WINF_WRITEABLE   0x80000000
#define WINF_ALIGN_NOPAD 0x00000008
#define WINF_ALIGN_BYTE  0x00100000
#define WINF_ALIGN_WORD  0x00200000
#define WINF_ALIGN_DWORD 0x00300000
#define WINF_ALIGN_8     0x00400000
#define WINF_ALIGN_PARA  0x00500000
#define WINF_ALIGN_32    0x00600000
#define WINF_ALIGN_64    0x00700000
#define WINF_ALIGN       (WINF_ALIGN_64)
#define WINF_COMMENT     0x00000200
#define WINF_REMOVE      0x00000800
#define WINF_COMDAT      0x00001000
#define WINF_NEG_FLAGS   (WINF_DISCARDABLE | WINF_NOPAGE)
#define WINF_IMAGE_FLAGS 0xfa0008e0

#define WIN32_DEFAULT_BASE              0x00400000
#define WIN32_DEFAULT_FILEALIGN         0x00000200
#define WIN32_DEFAULT_OBJECTALIGN       0x00001000
#define WIN32_DEFAULT_STACKSIZE         0x00100000
#define WIN32_DEFAULT_STACKCOMMITSIZE   0x00001000
#define WIN32_DEFAULT_HEAPSIZE          0x00100000
#define WIN32_DEFAULT_HEAPCOMMITSIZE    0x00001000
#define WIN32_DEFAULT_SUBSYS            PE_SUBSYS_WINDOWS
#define WIN32_DEFAULT_SUBSYSMAJOR       4
#define WIN32_DEFAULT_SUBSYSMINOR       0
#define WIN32_DEFAULT_OSMAJOR           1
#define WIN32_DEFAULT_OSMINOR           0

#define PE_FIXUP_ABSOLUTE 0
#define PE_FIXUP_HIGH 1
#define PE_FIXUP_LOW 2
#define PE_FIXUP_HIGHLOW 3
#define PE_FIXUP HIGHADJUST 4

#define LE_SIGNATURE 0x454C /* LE */
#define LX_SIGNATURE 0x584C /* LX */

#define LX_CPU_286 1
#define LX_CPU_386 2
#define LX_CPU_486 3

#define LX_OS_OS2 1
#define LX_OS_DOS 3

#define LX_MF_INTERNALFIXUPSAPPLIED   0x10
#define LX_MF_EXTERNALFIXUPSAPPLIED   0x20

#define LX_OF_READABLE   1
#define LX_OF_WRITEABLE  2
#define LX_OF_EXECUTABLE 4
#define LX_OF_RESOURCE 8
#define LX_OF_DISCARDABLE 0x10
#define LX_OF_SHARED 0x20
#define LX_OF_PRELOAD 0x40
#define LX_OF_INVALID 0x80
#define LX_OF_ZEROFILL 0x100
#define LX_OF_ALIASNEEDED 0x1000
#define LX_OF_BIGDEFAULT 0x2000
#define LX_OF_CONFORMINGCODE 0x4000
#define LX_OF_IOPRIV 0x8000


#define LE_OPF_ENUMERATED 0
#define LE_OPF_ITERATED 0x40
#define LE_OPF_INVALID 0x80
#define LE_OPF_ZERO 0xc0
#define LE_OPF_LASTPAGE 0x3

#define LX_OPF_ENUMERATED 0
#define LX_OPF_ITERATED 1
#define LX_OPF_INVALID 2
#define LX_OPF_ZERO 3
#define LX_OPF_RANGE 4

#define LX_FM_SOURCE_MASK 15
#define LX_FM_BYTE 0
#define LX_FM_SEL16 2
#define LX_FM_SEL16_PTR16 3
#define LX_FM_OFFSET16 5
#define LX_FM_SEL16_PTR32 6
#define LX_FM_OFFSET32 7
#define LX_FM_SELFREL32 8

#define LX_FM_FIXUPTOALIAS 0x10
#define LX_FM_SOURCELIST 0x20

#define LX_FF_TARGETTYPE_MASK 3

#define LX_FT_INTERNAL 0
#define LX_FT_IMPORTORDINAL 1
#define LX_FT_IMPORTNAME 2
#define LX_FT_IMPORTENTRY 3

#define LX_FF_ADDITIVE 4
#define LX_FF_TARGET32 0x10
#define LX_FF_ADDITIVE32 0x20
#define LX_FF_OBJMODORD16 0x40
#define LX_FF_ORD16 0x80