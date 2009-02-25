EXTRA_DIST = \
	augres.mc

if AUG_WIN32

bin_SCRIPTS = \
	augres.dll
CLEANFILES = \
	$(bin_SCRIPTS) \
	augres.h \
	augres.o \
	augres.rc \
	MSG00001.bin

augres.dll: augres.o
	dllwrap --dllname augres.dll --driver-name gcc $<

augres.o: augres.rc
	windres -o augres.o $<

augres.rc: augres.mc
	windmc $<

endif