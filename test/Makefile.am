EXTRA_DIST = \
	pytest1 \
	pytest2 \
	pytest3 \
	pytest4 \
	pytest5 \
	pytest6 \
	pytest7 \
	rbtest1 \
	rbtest2 \
	rbtest3 \
	rbtest4 \
	rbtest5 \
	rbtest6 \
	rbtest7


check_PROGRAMS =

TESTS_ENVIRONMENT = \
	AUG_LOGLEVEL=3 \
	AUG_BUILDDIR=$(top_builddir) \
	AUG_SRCDIR=$(top_srcdir)

TESTS =

if WITH_PYTHON
TESTS += \
	pytest1 \
	pytest2 \
	pytest3 \
	pytest4 \
	pytest5 \
	pytest6 \
	pytest7
endif

if WITH_RUBY
TESTS += \
	rbtest1 \
	rbtest2 \
	rbtest3 \
	rbtest4 \
	rbtest5 \
	rbtest6 \
	rbtest7
endif
