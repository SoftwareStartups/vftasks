# Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

COMPDIR = .
VFROOT := $(shell cd .. && pwd)
include $(VFROOT)/scripts/Make.defs

RESULT     := $(LIBDIR)/libvfstream.a
TGT_RESULT := $(VFSTREAM_LIBDIR)/libvfstream.a
CFLAGS     := -O3 -I$(INCDIR)

SRC        := $(wildcard $(SRCDIR)/*.c)
OBJ        := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o, $(SRC))
INC        := $(wildcard $(INCDIR)/*.h)
TGT_INC    := $(patsubst $(INCDIR)/%,$(VFSTREAM_INCDIR)/%, $(INC))

.PHONY : default all clean clean_all install

default all: $(RESULT)

install: $(TGT_RESULT) $(TGT_INC)

$(RESULT): $(OBJ) | $(LIBDIR)
	$(AR) cr $@ $^

$(TGT_RESULT): $(RESULT) | $(VFSTREAM_LIBDIR)
	$(UPDATE) $< $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(INC) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(VFSTREAM_INCDIR)/%.h: $(INCDIR)/%.h | $(VFSTREAM_INCDIR)
	$(UPDATE) $< $@

include $(VFROOT)/scripts/Make.rules
