COMPS := vfstream unit-test example
TESTS := unit-test example

.PHONY: default all test clean clean_all $(COMPS)

default all: $(COMPS)

$(COMPS):
	$(MAKE) -C $@

test:
	@for d in $(TESTS); do \
	  $(MAKE) -C $$d $@ ; \
	done
	
clean clean_all:
	@for d in $(COMPS); do \
	  $(MAKE) -C $$d $@ ; \
	done

unit-test: vfstream
example: vfstream
