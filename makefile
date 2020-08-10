CC = g++
CFLAGS = -fPIC
STARGET = liblogging.a
DTARGET = liblogging.so.1.10
OBJS = logging.o

all : $(STARGET) $(DTARGET) clean
.PHONY : all

$(STARGET) : $(OBJS)
	ar rcs $@ $^

$(DTARGET) : $(OBJS)
	$(CC) -shared -Wl,-soname,$(basename $(notdir $@)) -o $@ $^

-include $(OBJS:.o=.d)

%.o : %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%.d : %.cpp
	@$(CC) -MM $< > $@

clean:
	@rm *.o *.d 
