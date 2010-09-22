# Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

COMPS := vfstream unit-test example doc
TESTS := unit-test example
TAR_BASENAME := vfstream
RELEASE_COMPS := lib include
RELEASE_DIRS := $(addprefix $(VFSTREAMINSTALL)/,$(RELEASE_COMPS))
DEPLOY_FILES := \
	LICENSE.txt \
	README.txt \
	INSTALL.txt \
	doc \
	vfstream \
	example \
	Makefile \
	unit-test \
	scripts
	
.PHONY: default all install test release deploy clean clean_all $(COMPS)

default all: $(COMPS)

$(COMPS):
	$(MAKE) -C $@

install:
	$(MAKE) -C vfstream $@

test:
	@for d in $(TESTS); do \
	  $(MAKE) -C $$d $@ ; \
	done

release: $(RELEASE_DIRS)
	cd $(VFSTREAMINSTALL) && tar -czvf $(TAR_BASENAME).tgz $(RELEASE_COMPS)

deploy: $(COMPS)
	tar -czvf $(TAR_BASENAME).tar.gz \
		--exclude obj --exclude lib \
		--exclude-vcs \
		--exclude $(TAR_BASENAME).tar.gz $(DEPLOY_FILES)

clean:
	@for d in $(COMPS); do \
	  $(MAKE) -C $$d $@ ; \
	done

clean_all: clean
	rm -rf $(VFSTREAMINSTALL)

unit-test: vfstream
example: vfstream
