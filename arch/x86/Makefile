

.PHONY: x86 clean

SUBDIRS := boot video

x86: Makefile
	@for f in $(SUBDIRS); do \
		$(MAKE) $(MAKEFLAGS) -C $${f}; \
	done

clean:
	@for f in $(SUBDIRS); do \
		$(MAKE) clean  -C $${f}; \
	done
