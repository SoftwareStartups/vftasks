COMPDIR = .
VFROOT := $(shell cd .. && pwd)
include $(VFROOT)/scripts/Make.defs

CPPFLAGS := -O3 -pthread -I$(VFSTREAM_INCDIR)
LDFLAGS := -L$(VFSTREAM_LIBDIR) 

RESULT := unit_test

SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o, $(SRC))

.PHONY: default all test

default all: $(RESULT)

$(RESULT): $(OBJ)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) $^ -lvfstream -lcppunit -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) -c $< -o $@

test: $(RESULT)
	@./$^

include $(VFROOT)/scripts/Make.rules
