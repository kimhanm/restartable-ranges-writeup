CC := clang
#CFLAGS :=  -Wall -Wextra -Werror

main.out: main.c range.S
	$(CC) $(CFLAGS) $^ -o $@


.PHONY: inspect
inspect: main.out
	lldb --batch --source inspect.lldb main.out

.PHONY: clean
clean: 
	rm -f main.out
