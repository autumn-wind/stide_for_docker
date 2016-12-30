CC = g++
CPPFLAGS = -g -MD -I./include
CPPFILES = $(shell find src/ -name "*.C")
OBJS = $(CPPFILES:.C=.o)

run: $(OBJS)
	$(CC) -o run $(OBJS)

-include $(OBJS:.o=.d)

count:
	@echo total code lines \(empty lines not included\):
	@find . -name "*.[Ch]" | xargs grep "." | wc -l

clean:
	rm -f run $(OBJS) $(OBJS:.o=.d)


