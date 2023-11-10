# Main Makefile

# List of subdirectories to build
SUBDIRS := PHTMLLIB HTMLTEST POTADBUG PSWEEPER
SEND := false


.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS) $(SEND)

$(SUBDIRS):
	$(MAKE) -C $@ clean
	rm -f $@/src/gfx/*.c
	rm -f $@/src/gfx/*.h
	$(MAKE) -C $@ gfx
	$(MAKE) -C $@
	@if [ "$(SEND)" = "true" ]; then \
		tilp -s -n $@/bin/*.8xp; \
		tilp -s -n $@/bin/*.8xv; \
	fi 
	

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
		rm -f $$dir/src/gfx/*.c; \
		rm -f $$dir/src/gfx/*.h; \
	done \