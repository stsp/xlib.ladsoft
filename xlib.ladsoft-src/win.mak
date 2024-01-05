FILELOG = ..\libw.log
DEST = \tools\bin\win32
all:
        cd cc386
        echo CC386
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe > $(FILELOG)
        copy cc.exe $(DEST)\cc386.exe >> $(FILELOG)

        cd ..\nasm9838
        echo NASM
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy nasm.exe $(DEST) >> $(FILELOG)

        cd ..\valx
        echo VALX
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy valx.exe $(DEST) >> $(FILELOG)

        cd ..\imake
        echo IMAKE
        del imake.exe
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy imake.exe $(DEST) >> $(FILELOG)

        cd ..\xlib
        echo XLIB
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy xlib.exe $(DEST) >> $(FILELOG)

        cd ..\xrc
        echo XRC
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy xrc.exe $(DEST) >> $(FILELOG)

        cd ..\import
        echo IMPORT
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy import.exe $(DEST) >> $(FILELOG)

		cd ..\parsedll
		echo parser.dll
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
		copy parser.dll $(DEST)
	
        cd ..\ccide
        echo CCIDE
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy ccide.exe $(DEST) >> $(FILELOG)
        copy upgradedb.exe $(DEST) >> $(FILELOG)

        cd ..\cl_51
        echo CL386
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy cl386.exe $(DEST) >> $(FILELOG)

        cd ..\grep
        echo GREP
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy grep.exe $(DEST) >> $(FILELOG)

        cd ..\itouch
        echo ITOUCH
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy itouch.exe $(DEST) >> $(FILELOG)

        cd ..\mkliblst
        echo MKLIBLST
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy mkliblst.exe $(DEST) >> $(FILELOG)

        cd ..\mk_49
        echo MK386
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy mk386.exe $(DEST) >> $(FILELOG)

        cd ..\brc
        echo BRC
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.pe >> $(FILELOG)
        copy brc.exe $(DEST) >> $(FILELOG)

        cd ..