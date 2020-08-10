CC = g++
CFLAGS = -L .. -llogging -lpthread -static
TARGET = $(basename $(wildcard *.cpp))

all : $(TARGET) clean
.PHONY : all

-include $(addsuffix .d,$(TARGET))

% : %.cpp
	$(CC) -o $@ $< $(CFLAGS)

%.d : %.cpp
	@$(CC) -MM $< > $@.tmp; \
	sed "s/\($(basename $@)\).o/\1/g" < $@.tmp > $@; \
	rm $@.tmp

clean:
	@rm *.d
