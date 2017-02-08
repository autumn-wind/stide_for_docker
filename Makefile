CC = g++
CPPFLAGS = -g -MD -I./include
CPPFILES = $(shell find src/ -name "*.C")
OBJS = $(CPPFILES:.C=.o)

daemon: misc/daemon.C include/common.h monitor
	g++ misc/daemon.C -o daemon -I./include -std=c++11

monitor: $(OBJS)
	$(CC) -o monitor $(OBJS) -lpthread

-include $(OBJS:.o=.d)

count:
	@echo total code lines \(empty lines not included\):
	@find . -name "*.[Ch]" | xargs grep "." | wc -l

clean:
	rm -f daemon monitor $(OBJS) $(OBJS:.o=.d)


