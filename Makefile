PLATFORM := $(shell scripts/platform)

MAJOR := 1
MINOR := 0
BUILD := 0

BUILDDIR = build
LIB = $(BUILDDIR)/src/libvftasks.a
INC = include/vftasks.h
INSTALL_INCDIR = $(VFTASKSINSTALL)/include
INSTALL_LIBDIR = $(VFTASKSINSTALL)/$(PLATFORM)/lib

.PHONY: default all clean clean_all install release check_env

default all: install

install: check_env | $(BUILDDIR) $(INSTALL_INCDIR) $(INSTALL_LIBDIR)
	(cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=release .. \
	  -DINSTALLDIR=$(VFTASKSINSTALL) -DMAJOR=$(MAJOR) -DMINOR=$(MINOR) -DBUILD=$(BUILD) \
	  -DPACKAGENAME=vftasks && make vftasks)
	cp -u $(LIB) $(INSTALL_LIBDIR)
	cp -u $(INC) $(INSTALL_INCDIR)

test: | $(BUILDDIR)
	(cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=debug ..)
	make -C $(BUILDDIR) unit_test run_test

release: install
	$(MAKE) -C $(BUILDDIR) package
	cp $(BUILDDIR)/vftasks$(MAJOR)$(MINOR)-$(MAJOR).$(MINOR).$(BUILD).deb $(VFTASKSINSTALL)
	(cd $(VFTASKSINSTALL) && tar -czf vftasks.tgz $(PLATFORM) include)

$(BUILDDIR) $(INSTALL_INCDIR) $(INSTALL_LIBDIR):
	mkdir -p $@

check_env:
	@if [ -z "$$VFTASKSINSTALL" ]; then \
	  echo "Error: you must set the VFTASKSINSTALL environment variable"; \
	  exit 1; \
	fi;

clean:
	rm -rf $(BUILDDIR)

clean_all: check_env clean
	rm -rf $(VFTASKSINSTALL)/* result.xml
