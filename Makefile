CC = g++
CPPFLAGS = -g -MD -I./include
CPPFILES = $(shell find src/ -name "*.C")
OBJS = $(CPPFILES:.C=.o)

#daemon: src/daemon.C include/common.h src/monitor monitor
	#g++ src/daemon.C -o src/daemon -I./include -std=c++11
	#docker events --filter 'event=start' | sudo src/daemon

#monitor: src/monitor.C include/common.h
	#g++ src/monitor.C -o src/monitor -I./include -lpthread -std=c++11

monitor: $(OBJS)
	$(CC) -o monitor $(OBJS) -lpthread

-include $(OBJS:.o=.d)

count:
	@echo total code lines \(empty lines not included\):
	@find . -name "*.[Ch]" | xargs grep "." | wc -l

clean:
	rm -f monitor $(OBJS) $(OBJS:.o=.d)


