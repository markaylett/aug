SUBDIRS = \
	man

EXTRA_DIST = \
	daug.txt \
	daugintro.w \
	cvs-policy.txt \
	cwebmac.tex \
	Doxyfile

dotdepend:
	for l in augsys augmar augutil augsrv augnet; do \
		(cd $(top_srcdir)/src/c && \
			../pl/dotdepend -I. `find $$l -name "*.c" | grep -v '[.]libs'`) \
			| dot -Tgif >$$l.gif; \
	done

modsample.cpp: daugintro.w
	ctangle $< - $@

daugintro.tex: daugintro.w
	cweave $<

daugintro.dvi: daugintro.tex
	tex "\let\pdf+ \input $<"

daugintro.pdf: daugintro.dvi
	dvipdfm $<
