BUILDDIR = build
LIB = $(BUILDDIR)/src/libvftasks.a
INC = include/vftasks.h
INSTALL_INCDIR = $(VFTASKSINSTALL)/tools/include
INSTALL_LIBDIR = $(VFTASKSINSTALL)/tools/lib

.PHONY: default all clean clean_all install release check_env

default all: install

install: check_env | $(BUILDDIR) $(INSTALL_INCDIR) $(INSTALL_LIBDIR)
	cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=release .. && make vftasks
	cp -u $(LIB) $(INSTALL_LIBDIR)
	cp -u $(INC) $(INSTALL_INCDIR)

test: | $(BUILDDIR)
	cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=debug ..
	-cd $(BUILDDIR) && make unit_test run_test

release: install
	cd $(VFTASKSINSTALL)/tools && tar -czf vftasks.tgz include lib

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
