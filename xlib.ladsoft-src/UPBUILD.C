 #include <stdio.h>

int transform(char *buf, char *str,char*ver)
{
   char * p ;
   int val ;
   if (strstr(buf,str)) {
      p = strrchr(buf,'.') ;
      if (!p)
         return 0 ;
      val = atoi(p+1) ;
      sprintf(p+1,"%d\"\n",val+1) ;
   } else if (strstr(buf,ver)) {
      p = strrchr(buf,',') ;
      if (!p)
         return 0 ;
      val = atoi(p+1) ;
      sprintf(p+1,"%d\n",val+1) ;
   } else
      return 0 ;
}
main(int argc, char *argv[])
{
   FILE *fil ;
   FILE *out ;
   char str[256],ver[256], filname[256],*p ;
   int done = 0 ;
   if (argc != 2) {
      return 0 ;
   }
   strcpy(filname,argv[0]);
   p = strrchr(filname,'\\');
   strcpy(p,"\\version.h");
   printf("%s",filname);
   fil = fopen(filname,"r") ;
   out = fopen("$$TEMP.TMP","w") ;
   if (!fil) {
      printf("Can't find version file") ;
      fclose(out) ;
      return 1 ;
   }
   sprintf(str,"%s_STRING_VERSION",argv[1]) ;
   sprintf(ver,"%s_VERSION",argv[1]) ;
   while (!feof(fil)) {
      char buf[256] ;
      buf[0] = 0 ;
      fgets(buf,256,fil) ;
      done |= transform(buf,str,ver) ;
      fputs(buf,out) ;
   }
   fclose(fil) ;
   fclose(out) ;
   if (!done) {
      printf("unknown parameter") ;
      unlink("$$TEMP.TMP") ;
      return 1 ;
   }
   unlink(filname) ;
   rename("$$TEMP.TMP",filname) ;

   return 0 ;
}