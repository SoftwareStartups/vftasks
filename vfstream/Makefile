COMPDIR = .
VFROOT := $(shell cd .. && pwd)
include $(VFROOT)/scripts/Make.defs

RESULT := $(LIBDIR)/libvfstream.a
CFLAGS := -O3 -I$(INCDIR)

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o, $(SRC))

.PHONY : clean clean_all

default all: $(RESULT)

$(RESULT): $(OBJ) | $(LIBDIR)
	$(AR) cr $@ $^

$(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

include $(VFROOT)/scripts/Make.rules
