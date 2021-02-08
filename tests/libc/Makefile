.NOTPARALLEL:

TOP = $(abspath ../..)
include $(TOP)/defs.mak

IMG=myst-alpine

ifdef STRACE
OPTS = --strace
endif

ROOTHASH=$(SUBOBJDIR)/roothash

APPDIR=$(CURDIR)/appdir

ROOTFS=$(CURDIR)/rootfs

ifdef TEST
export TIMEOUT=5
# OPTS += --strace
one:
	$(RUNTEST) $(MYST_EXEC) $(OPTS) $(ROOTFS) $(TEST)
endif

ifdef FAILED
  TEST_FILE = failed.txt
  export TIMEOUT=10
else
  TEST_FILE = alltests.txt
endif

TESTS = $(shell cat $(TEST_FILE))

##==============================================================================
##
## all rule:
##
##==============================================================================

all: $(APPDIR) $(APPDIR)/target $(APPDIR)/bin/run $(ROOTFS) $(ROOTHASH)

$(APPDIR):
	rm -rf $(APPDIR)
	cp -r $(TOP)/third_party/libc-test/libc-test $(APPDIR)
	cp $(CURDIR)/alltests.txt $(APPDIR)
	cp $(CURDIR)/failed.txt $(APPDIR)

$(APPDIR)/target:
	sudo docker run --rm -v $(APPDIR):/appdir $(IMG) bash -c "make -j -C appdir"
	touch $(APPDIR)/target

$(APPDIR)/bin/run: run.c
	mkdir -p $(APPDIR)/bin
	$(MUSL_GCC) -Wall -o $(APPDIR)/bin/run run.c

$(ROOTFS): run.c
	sudo $(MYST) mkext2 --force $(APPDIR) $(ROOTFS)

$(ROOTHASH): run.c
	mkdir -p $(SUBOBJDIR)
	$(MYST) fssig --roothash $(ROOTFS) > $(ROOTHASH)

##==============================================================================
##
## tests:
##
##==============================================================================

RUN = $(RUNTEST) $(MYST_EXEC) $(OPTS) --roothash=$(ROOTHASH) $(ROOTFS)

OPTS += --roothash=$(ROOTHASH)

tests: all
	$(RUNTEST) $(MYST_EXEC) $(OPTS) $(ROOTFS) /bin/run $(STATUS) $(TEST_FILE)

failedtests: all
	@ $(foreach i, $(TESTS), TESTNAME=$(TESTNAME)$(i)$(TESTSUFFIX) $(RUNTEST) $(MYST_EXEC) $(OPTS) $(ROOTFS) $(TEST) $(i) $(NL) )

clean:
	rm -rf $(APPDIR) $(ROOTFS) $(ROOTHASH)
