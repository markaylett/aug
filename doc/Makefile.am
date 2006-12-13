SUBDIRS = \
	man

EXTRA_DIST = \
	augas.txt \
	augsys.gif \
	augmar.gif \
	augutil.gif \
	augsrv.gif \
	augnet.gif \
	cvs-policy.txt \
	Doxyfile

doc:
	for l in augsys augmar augutil augsrv augnet; do \
		(cd $(top_srcdir)/src/c && \
			../pl/dotdepend -I. `find $$l -name "*.c"`) \
			| dot -Tgif >$$l.gif; \
	done
