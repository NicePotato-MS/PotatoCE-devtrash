# ----------------------------
# Makefile Options
# ----------------------------

NAME = TMUXCE
ICON = icon.png
DESCRIPTION = "NicePotato's Tmux/Bash"
COMPRESSED = YES
COMPRESSED_MODE = zx7
LTO = YES
ARCHIVED = YES
HAS_PRINTF = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# font options
FONTPACKNAME = DRMONO
FONTDIR = $(SRCDIR)/fonts

DEPS = $(BINDIR)/$(FONTPACKNAME).bin $(BINDIR)/$(FONTPACKNAME).8xv

# ----------------------------

include $(shell cedev-config --makefile)

# ----------------------------

$(BINDIR)/$(FONTPACKNAME).bin: $(FONTDIR)/drmono-10-normal.convfont $(FONTDIR)/drmono-10-bold.convfont $(FONTDIR)/drmono-14-normal.convfont $(FONTDIR)/drmono-14-bold.convfont
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o fontpack -N "Dr. Mono" -P "Calculator 1252" \
	-D "A monospaced variant of Dr. Sans" \
	-V "25 May 2020" \
	-t $(FONTDIR)/drmono-10-normal.convfont -l 8  -x 3 -c 1 -a 1 -b 1 -w normal -s 0 -s sans-serif -s upright -s monospaced \
	-t $(FONTDIR)/drmono-10-bold.convfont   -l 8  -x 3 -c 1 -a 1 -b 1 -w bold -s 0 -s sans-serif -s upright -s monospaced \
	-t $(FONTDIR)/drmono-14-normal.convfont -l 11 -x 5 -c 2 -a 1 -b 1 -w normal -s 0 -s sans-serif -s upright -s monospaced \
	-t $(FONTDIR)/drmono-14-bold.convfont   -l 11 -x 5 -c 2 -a 1 -b 1 -w bold -s 0 -s sans-serif -s upright -s monospaced \
	$(BINDIR)/$(FONTPACKNAME).bin

$(BINDIR)/$(FONTPACKNAME).8xv: $(BINDIR)/$(FONTPACKNAME).bin
	$(Q)$(call MKDIR,$(@D))
	$(Q)convbin --archive --name $(FONTPACKNAME) --iformat bin --oformat 8xv --input $(BINDIR)/$(FONTPACKNAME).bin --output $(BINDIR)/$(FONTPACKNAME).8xv