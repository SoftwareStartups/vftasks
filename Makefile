BUILDDIR = build

.PHONY: default all clean clean_all install release

default all: install

install: $(BUILDDIR)
	cd $(BUILDDIR) && cmake .. && make install DESTDIR=$(VFSTREAMINSTALL)

test: install
	cd $(BUILDDIR) && make test

release: install
	cd $(BUILDDIR) && cmake .. && make package && cp vftasks.tar.gz $(VFSTREAMINSTALL)/vfstream.tgz

$(BUILDDIR):
	mkdir -p $@
clean:
	rm -rf $(BUILDDIR)

clean_all: clean
	rm -rf $(VFSTREAMINSTALL)/*
