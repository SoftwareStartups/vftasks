PLATFORM := $(shell scripts/platform)

MAJOR_VERSION ?= 0
MINOR_VERSION ?= 0
BUILD_VERSION ?= 0
VERSION := $(MAJOR_VERSION).$(MINOR_VERSION).$(BUILD_VERSION)

BUILDDIR = build

.PHONY: default all clean clean_all install release check_env

default all: install

install: check_env | $(BUILDDIR)
	(cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=release .. \
	  -DCMAKE_INSTALL_PREFIX=$(VFTASKSINSTALL) \
	  -DCMAKE_LIBRARY_ARCHITECTURE=$(PLATFORM) \
	  -DMAJOR=$(MAJOR_VERSION) -DMINOR=$(MINOR_VERSION) -DBUILD=$(BUILD_VERSION) \
	  -DPACKAGENAME=vftasks && make install package)
	cp $(BUILDDIR)/vftasks$(MAJOR_VERSION)$(MINOR_VERSION)-$(VERSION).deb $(VFTASKSINSTALL)

test: | $(BUILDDIR)
	(cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=debug ..)
	make -C $(BUILDDIR) unit_test run_test

release: install
	(cd $(VFTASKSINSTALL) && tar -czf vftasks.tgz lib include \
	  vftasks$(MAJOR_VERSION)$(MINOR_VERSION)-$(VERSION).deb)

$(BUILDDIR):
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
