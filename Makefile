PLATFORM := $(shell scripts/platform)

MAJOR_VERSION ?= 0
MINOR_VERSION ?= 0
BUILD_VERSION ?= 0
VERSION := $(MAJOR_VERSION).$(MINOR_VERSION).$(BUILD_VERSION)

BUILDDIR = build
DEB_FILENAME := vftasks$(MAJOR_VERSION)$(MINOR_VERSION)_$(VERSION)-1_amd64.deb
RPM_FILENAME := vftasks$(MAJOR_VERSION)$(MINOR_VERSION)-$(VERSION)-1.x86_64.rpm

# VFA-2777: on Ubuntu 12.04 (cmake 2.8.7) fakeroot is implicit, whereas on older
# versions, it needs to be prefixed explicitly.
UBUNTU_VERSION := $(shell lsb_release -sr)
ifeq ($(UBUNTU_VERSION),12.04)
FAKEROOT :=
else ifeq ($(UBUNTU_VERSION),12.10)
FAKEROOT :=
else
FAKEROOT := fakeroot
endif

.PHONY: default all clean clean_all install release check_env

default all: install

install: check_env | $(BUILDDIR)
	(cd $(BUILDDIR) && cmake -Wdev -DCMAKE_BUILD_TYPE=release .. \
	  -DCMAKE_INSTALL_PREFIX=$(VFTASKSINSTALL) \
	  -DCMAKE_LIBRARY_ARCHITECTURE=$(PLATFORM)-gnu \
	  -DMAJOR=$(MAJOR_VERSION) -DMINOR=$(MINOR_VERSION) -DBUILD=$(BUILD_VERSION) \
	  -DPACKAGENAME=vftasks && \
	  $(MAKE) install && \
	  $(FAKEROOT) $(MAKE) package)
	cp $(BUILDDIR)/vftasks$(MAJOR_VERSION)$(MINOR_VERSION)-$(VERSION).deb $(VFTASKSINSTALL)/$(DEB_FILENAME)
	cp $(BUILDDIR)/vftasks$(MAJOR_VERSION)$(MINOR_VERSION)-$(VERSION).rpm $(VFTASKSINSTALL)/$(RPM_FILENAME)

test: | $(BUILDDIR)
	(cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=debug ..)
	$(MAKE) -C $(BUILDDIR) unit_test run_test

release: install
	(cd $(VFTASKSINSTALL) && tar -czf vftasks.tgz lib include \
	  $(DEB_FILENAME) $(RPM_FILENAME))

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
