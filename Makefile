BUILDDIR = build

.PHONY: default all clean clean_all install release

default all: install

install: | $(BUILDDIR)
	cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=release .. && make install DESTDIR=$(VFSTREAMINSTALL)

test: | $(BUILDDIR)
	cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=debug .. && make all test

release: install
	cd $(BUILDDIR) && cmake .. && make package && cp vftasks.tar.gz $(VFSTREAMINSTALL)/vfstream.tgz

$(BUILDDIR):
	mkdir -p $@
clean:
	rm -rf $(BUILDDIR)

clean_all: clean
	rm -rf $(VFSTREAMINSTALL)/* result.xml
