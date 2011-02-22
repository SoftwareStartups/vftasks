# Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

COMPDIR = .
VFROOT := $(shell cd .. && pwd)
include $(VFROOT)/scripts/Make.defs

TGT_LIBDIR := $(VFSTREAMINSTALL)/lib
TGT_INCDIR := $(VFSTREAMINSTALL)/include

SRC     := $(wildcard $(SRCDIR)/*.c)
OBJ     := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o, $(SRC))
INC     := $(wildcard $(INCDIR)/*.h)

RESULT  := $(LIBDIR)/libvftasks.a
TGT_LIB := $(TGT_LIBDIR)/libvftasks.a
TGT_INC := $(patsubst $(INCDIR)/%,$(TGT_INCDIR)/%, $(INC))
CFLAGS  := -O3 -I$(INCDIR)

.PHONY : default all clean clean_all install

default all: $(RESULT)

install: $(TGT_LIB) $(TGT_INC)

$(RESULT): $(OBJ) | $(LIBDIR)
	$(AR) cr $@ $^

$(TGT_LIB): $(RESULT) | $(TGT_LIBDIR)
	$(UPDATE) $< $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(INC) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TGT_INCDIR)/%.h: $(INCDIR)/%.h | $(TGT_INCDIR)
	$(UPDATE) $< $@

include $(VFROOT)/scripts/Make.rules
