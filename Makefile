# Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

COMPS := vfstream unit-test example doc
TESTS := unit-test example
RELEASE := vfstream
RELEASE_FILES := \
	LICENSE.txt \
	README.txt \
	INSTALL.txt \
	doc \
	vfstream \
	example \
	Makefile \
	unit-test \
	scripts
	
.PHONY: default all install test release clean clean_all $(COMPS)

default all: $(COMPS)

$(COMPS):
	$(MAKE) -C $@

install:
	$(MAKE) -C vfstream $@

test:
	@for d in $(TESTS); do \
	  $(MAKE) -C $$d $@ ; \
	done

release: $(COMPS)
	tar -czvf $(RELEASE).tar.gz \
		--exclude obj --exclude lib \
		--exclude-vcs \
		--exclude $(RELEASE).tar.gz $(RELEASE_FILES)

clean clean_all:
	@for d in $(COMPS); do \
	  $(MAKE) -C $$d $@ ; \
	done

unit-test: vfstream
example: vfstream
