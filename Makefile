# Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

COMPS := vfstream unit-test example doc
TESTS := unit-test example
VERSION := 1.0
RELEASE := vfstream-$(VERSION)

.PHONY: default all test release clean clean_all $(COMPS)

default all: $(COMPS)

$(COMPS):
	$(MAKE) -C $@

test:
	@for d in $(TESTS); do \
	  $(MAKE) -C $$d $@ ; \
	done

releas: all
	tar -czvf $(RELEASE).tar.gz \
		--exclude obj --exclude lib \
		--exclude .hg --exclude .hgignore \
		--exclude $(RELEASE).tar.gz .

clean clean_all:
	@for d in $(COMPS); do \
	  $(MAKE) -C $$d $@ ; \
	done

unit-test: vfstream
example: vfstream
