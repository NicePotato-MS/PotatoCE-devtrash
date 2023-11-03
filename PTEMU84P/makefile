# ----------------------------
# Makefile Options
# ----------------------------

NAME = PTEMU84P
ICON = icon.png
DESCRIPTION = "."
COMPRESSED = NO
ARCHIVED = YES
HAS_PRINTF = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz
# font options
FONTPACKNAME = DRSANS
FONTDIR = $(SRCDIR)/fonts
MAKE_GFX = cd $(GFXDIR) && $(CONVIMG)

DEPS = $(BINDIR)/$(FONTPACKNAME).bin $(BINDIR)/$(FONTPACKNAME).8xv

# ----------------------------

include $(shell cedev-config --makefile)

# ----------------------------

