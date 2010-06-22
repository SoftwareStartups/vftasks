COMPDIR = .
VFROOT := $(shell cd .. && pwd)
include $(VFROOT)/scripts/Make.defs

RESULT = partition

CFLAGS := -pthread -O3 -I$(VFSTREAM_INCDIR)
LDFLAGS := -L$(VFSTREAM_LIBDIR)

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

.PHONY: default all test

default all: $(RESULT)

$(RESULT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -lvfstream -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(RESULT)
	@./$<
	
include $(VFROOT)/scripts/Make.rules
