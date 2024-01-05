
FILELOG = ..\libb.log
DEST = \tools\bin\win32
all:
        cd OBJECT
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 > $(FILELOG)
        copy cc.exe $(DEST)\cc386.exe >> $(FILELOG)

        cd ..\nasm9838
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy nasm.exe $(DEST) >> $(FILELOG)

        cd ..\valx
        del *.asm *.obj *.res valx.exe
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy valx.exe $(DEST) >> $(FILELOG)

        cd ..\imake
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy imake.exe $(DEST) >> $(FILELOG)

        cd ..\xlib
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy xlib.exe $(DEST) >> $(FILELOG)

        cd ..\xrc
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy xrc.exe $(DEST) >> $(FILELOG)

        cd ..\ccide
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy ccide.exe $(DEST) >> $(FILELOG)

        cd ..\cl_51
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy cl386.exe $(DEST) >> $(FILELOG)

        cd ..\grep
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy grep.exe $(DEST) >> $(FILELOG)

        cd ..\itouch
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy itouch.exe $(DEST) >> $(FILELOG)

        cd ..\mkliblst
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy mkliblst.exe $(DEST) >> $(FILELOG)

        cd ..\mk_49
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy mk386.exe $(DEST) >> $(FILELOG)

        cd ..\import
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy import.exe $(DEST) >> $(FILELOG)

        cd ..\brc
        del *.asm *.obj *.res
        \bc5\bin\make -DFULLBUILD=FULLBUILD  -fmakefile.b32 >> $(FILELOG)
        copy brc.exe $(DEST) >> $(FILELOG)

        cd ..