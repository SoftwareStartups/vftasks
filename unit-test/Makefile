# Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

COMPDIR = .
VFROOT := $(shell cd .. && pwd)
include $(VFROOT)/scripts/Make.defs

CPPFLAGS := -O3 -pthread -I$(VFSTREAM_INCDIR) -I$(VFTASKS_INCDIR)
LDFLAGS := -L$(VFSTREAM_LIBDIR) -L$(VFTASKS_LIBDIR)

RESULT := unit_test

SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o, $(SRC))

.PHONY: default all test

default all: $(RESULT)

$(RESULT): $(OBJ)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) $^ -lvfstream -lvftasks -lcppunit -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CPPFLAGS) -c $< -o $@

test: $(RESULT)
	@./$^

clean_all::
	rm -f result.xml

include $(VFROOT)/scripts/Make.rules
