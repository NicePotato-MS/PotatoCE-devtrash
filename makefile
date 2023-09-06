# Main Makefile

# List of subdirectories to build
SUBDIRS := PHTMLLIB HTMLTEST POTADBUG
SEND := false


.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS) $(SEND)

$(SUBDIRS):
	$(MAKE) -C $@
	@if [ "$(SEND)" = "true" ]; then \
		tilp -s -n $@/bin/*.8xp; \
		tilp -s -n $@/bin/*.8xv; \
	fi 
	

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done