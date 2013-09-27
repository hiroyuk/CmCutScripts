ASM   = ml
AFLAG = /c /coff
# AFLAG = /c /coff /Zd /Zi
# CC = cl
# CFLAG = /c /Gr /GF /Oa /Ob2 /Ot /Og /Oi /Op /ML /W4 /Zp1 /D "NDEBUG"
CC = icl
CFLAG = /c /Zp1 /D "NDEBUG" /Oy /Qsfalign16 /Qsox- /Qvc6 /Zp1 /G6 /Og /Oi /Op /Ot /Qip
LINK  = link
LFLAG = /opt:ref /nologo /out:mme.exe /subsystem:windows user32.lib gdi32.lib advapi32.lib comdlg32.lib comctl32.lib shell32.lib
RC    = rc

OBJ   = mme.obj m2v_vfapi.obj mme_project.obj callback.obj query_frame.obj timecode.obj message.obj
RES   = mme.res

ALL: mme

mme: $(OBJ) $(RES)
	$(LINK) $(LFLAG) *.obj $(RES)

mme.obj: mme.c resource.h m2v_vfapi.h mme_project.h vfapi.h vfapi_edit_ext.h query_frame.h message.h
	$(CC) $(CFLAG) mme.c

m2v_vfapi.obj: m2v_vfapi.c m2v_vfapi.h vfapi.h vfapi_edit_ext.h
	$(CC) $(CFLAG) m2v_vfapi.c

mme_project.obj: mme_project.c mme_project.h vfapi.h vfapi_edit_ext.h timecode.h message.h
	$(CC) $(CFLAG) mme_project.c

callback.obj: callback.c callback.h resource.h
	$(CC) $(CFLAG) callback.c

query_frame.obj: query_frame.c query_frame.h resource.h timecode.h message.h
	$(CC) $(CFLAG) query_frame.c

timecode.obj: timecode.c timecode.h
	$(CC) $(CFLAG) timecode.c

message.obj: message.c message.h
	$(CC) $(CFLAG) message.c

mme.res: mme.rc resource.h
	$(RC) mme.rc

clean:
	DEL *.obj
	DEL *.exe
	DEL *.res