
.PHONY: all clean
all: 
	@echo "nop"

clean:
	for dir in */ ; do \
		$(MAKE) -C $$dir clean ; \
	done
