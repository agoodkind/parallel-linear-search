all:
	$(error you must specificy a target. Either 'proc', 'thread', or 'linear')

CFLAGS=-fsanitize=address -std=gnu11 -g -lpthread -lm -w -O0
POSTMAKE=chmod +x a.out

proc: proc.o
	gcc searchtest.c -D PROC $(CFLAGS) multitest_proc.o
	$(POSTMAKE)

thread: thread.o
	gcc searchtest.c -D THREAD $(CFLAGS) multitest_thread.o 
	$(POSTMAKE)

linear: 
	gcc searchtest.c -D LINEAR $(CFLAGS)
	$(POSTMAKE)

proc.o:
	gcc -o multitest_proc.o -c multitest_proc.c $(CFLAGS)

thread.o:
	gcc -o multitest_thread.o -c multitest_thread.c $(CFLAGS)


clean:
	rm -rf *.a; rm -rf *.o; rm -rf *.out; rm -rf *.dSYM;
