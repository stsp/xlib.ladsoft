FILELOG = ..\libd.log
DEST = \tools\bin
all:
        cd cc386
        echo cc386
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le > $(FILELOG)
        copy cc.exe $(DEST)\cc386.exe >> $(FILELOG)

        cd ..\nasm9838
        echo NASM
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy nasm.exe $(DEST) >> $(FILELOG)

        cd ..\valx
        echo VALX
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy valx.exe $(DEST) >> $(FILELOG)

        cd ..\imake
        echo IMAKE
        del imake.exe
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy imake.exe $(DEST) >> $(FILELOG)

        cd ..\xlib
        echo XLIB
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy xlib.exe $(DEST) >> $(FILELOG)

        cd ..\cl_51
        echo CL386
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy cl386.exe $(DEST) >> $(FILELOG)

        cd ..\grep
        echo GREP
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy grep.exe $(DEST) >> $(FILELOG)

        cd ..\itouch
        echo ITOUCH
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy itouch.exe $(DEST) >> $(FILELOG)

        cd ..\mkliblst
        echo MKLIBLST
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy mkliblst.exe $(DEST) >> $(FILELOG)

        cd ..\mk_49
        echo MK386
        imake -B -DFULLBUILD=FULLBUILD  -fmakefile.le >> $(FILELOG)
        copy mk386.exe $(DEST) >> $(FILELOG)

		cd ..\uz
		echo UZ
		cc386 /Wa uz.c >> $(FILELOG)
		copy uz.exe $(DEST) >> $(FILELOG)

        cd ..