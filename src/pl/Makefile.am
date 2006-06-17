AUG_MAR = Aug-Mar-0.1

EXTRA_DIST = \
	$(AUG_MAR).tar.gz \
	dotdepend

dist-hook:
	if test -d $(AUG_MAR); then \
      (cd $(AUG_MAR) \
        && AUG_HOME=$(prefix) perl Makefile.PL \
        && rm -f $(AUG_MAR).tar.gz \
        && make dist \
        && mv $(AUG_MAR).tar.gz ../); \
    fi
