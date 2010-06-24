
# Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

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
