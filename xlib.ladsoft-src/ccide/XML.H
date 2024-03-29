/* 
CCIDE
Copyright 2001-2011 David Lindauer.

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

You may contact the author at:
	mailto::camille@bluegrass.net
 */
#define MAX_XML_TEXT_LEN 0x800
#define MAX_XML_NAME_LEN 0x80
struct xmlAttr
{
    struct xmlAttr *next;
    char *name;
    char *value;
};

struct xmlNode
{
    struct xmlNode *next;
    char *elementType;
    char *textData;
    struct xmlAttr *attribs;
    struct xmlNode *children;
    struct xmlNode *parent;
};

void xmlFree(struct xmlNode *Node);
struct xmlNode *xmlReadFile(FILE *in);
void xmlWriteFile(FILE *out, struct xmlNode *root);
char *xmlConvertString(char *s, int attr);
