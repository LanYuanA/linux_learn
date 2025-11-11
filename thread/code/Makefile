CC := /usr/bin/gcc
thread_pool_test: thread_pool_test.c
	-$(CC) -o $@ $^ `pkg-config --cflags --libs glib-2.0`
	-./$@
	-rm ./$@