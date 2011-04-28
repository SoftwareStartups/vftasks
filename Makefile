BUILDDIR = build

.PHONY: default all clean clean_all install release check_env

default all: install

install: | $(BUILDDIR)
	cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=release .. && make install DESTDIR=$(VFTASKSINSTALL)

test: | $(BUILDDIR)
	cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=debug ..
	-cd $(BUILDDIR) && make all test

release: install
	cd $(BUILDDIR) && cmake .. && make package && cp vftasks.tar.gz $(VFTASKSINSTALL)/vfstream.tgz

$(BUILDDIR):
	mkdir -p $@

check_env:
	@if [ -z "$$VFTASKSINSTALL" ]; then \
	  echo "Error: you must set the VFTASKSINSTALL environment variable"; \
	  exit 1; \
	fi;

clean: check_env
	rm -rf $(BUILDDIR)

clean_all: clean
	rm -rf $(VFTASKSINSTALL)/* result.xml
