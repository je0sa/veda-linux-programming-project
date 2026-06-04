SUBDIRS = client device server

all:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done


clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	rm -rf bin

.PHONY: all clean $(SUBDIRS)