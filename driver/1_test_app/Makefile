objs = a.o b.o c.o
dep_files := $(patsubst %,.%.d,$(objs))
dep_files :=  $(wildcard $(dep_files))

CFLAGS = -Werror -Iinclude

test : $(objs)
	gcc -o test $^
ifneq ($(dep_files),)
-include $(dep_files)
endif

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $< -MD -MF .$@.d

clean:
	rm -f *.o test
distclean: 
	rm -f $(dep_files)
.PHONY: clean distclean
