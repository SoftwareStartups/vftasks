COMPS := vfstream unit-test example doc
TESTS := unit-test example

.PHONY: default all test doc clean clean_all $(COMPS)

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
