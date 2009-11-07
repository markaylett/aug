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
	$(DLLWRAP) --as $(AS) --dllname augres.dll --driver-name $(CC) $<

augres.o: augres.rc
	$(WINDRES) -o augres.o $<

augres.rc: augres.mc
	$(WINDRC) $<

endif
