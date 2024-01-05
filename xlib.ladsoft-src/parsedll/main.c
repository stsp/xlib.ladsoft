#include <stdio.h>
extern void parser_main(char *, char *);

int main(int argc, char* argv[])
{
	if (argv[1])
	{
		FILE *fp = fopen(argv[1],"rb");
		if (fp)
		{
			long l;
			char *str;
			fseek(fp, 0, SEEK_END);
			l = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			str = calloc(1+1, l);
			if (str)
			{
				fread(str, 1, l, fp);
		        parser_main(argv[1],str);
				free(str);
			}
			fclose(fp);
		}
	}
    return 0;
}
