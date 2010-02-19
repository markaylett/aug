EXTRA_DIST = \
	augres.mc \
	augres.rc \
	MSG00001.bin

if AUG_WIN32

bin_SCRIPTS = \
	augres.dll
CLEANFILES = \
	$(bin_SCRIPTS) \
	augres.h \
	augres.o

augres.dll: augres.o
	TMPDIR=c:/temp $(DLLWRAP) --as $(AS) --dllname augres.dll --driver-name $(CC) $<

augres.o: augres.rc
	TMPDIR=c:/temp $(WINDRES) -o augres.o $<

augres.rc: augres.mc
	TMPDIR=c:/temp $(WINDRC) $<

endif
