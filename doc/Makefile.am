SUBDIRS = \
	man

EXTRA_DIST = \
	augas.txt \
	augasintro.w \
	augsys.gif \
	augmar.gif \
	augutil.gif \
	augsrv.gif \
	augnet.gif \
	cvs-policy.txt \
	cwebmac.tex \
	Doxyfile

gif:
	for l in augsys augmar augutil augsrv augnet; do \
		(cd $(top_srcdir)/src/c && \
			../pl/dotdepend -I. `find $$l -name "*.c"`) \
			| dot -Tgif >$$l.gif; \
	done

augasintro.tex: augasintro.w
	cweave $<

augasintro.dvi: augasintro.tex
	tex $<

augasintro.pdf: augasintro.dvi
	dvipdfm $<
