.PHONY: test

CFLAGS=-I ../src -Wall -g -DSENTINEL -fsanitize=address
LDFLAGS=-fsanitize=address

test: test-rbtree
	./test-rbtree
	$(CC) $(CFLAGS) $(LDFLAGS) test-rbtree.o ../src/rbtree.o -o test-rbtree
# 	valgrind ./test-rbtree

test-rbtree: test-rbtree.o ../src/rbtree.o

../src/rbtree.o:
	$(MAKE) -C ../src rbtree.o

clean:
	rm -f test-rbtree *.o ../src/rbtree.o

