CC=gcc
CFLAGS=-g -Wall -lpthread
SRCDIR=src
BINDIR=bin

all: bench1 bench2 bench3

bench1: sync
	$(CC) $(CFLAGS) $(SRCDIR)/bench1.c $(BINDIR)/sync.o -o $(BINDIR)/bench1

run-bench1: bench1
	@echo "Run 1:"
	bash -c 'time $(BINDIR)/bench1 -t 1 -l 1 -m 2000'
	@echo ""
	@echo "Run 2:"
	bash -c 'time $(BINDIR)/bench1 -t 2 -l 1 -m 2000'
	@echo ""
	@echo "Run 3:"
	bash -c 'time $(BINDIR)/bench1 -t 4 -l 1 -m 2000'
	@echo ""
	@echo "Run 4:"
	bash -c 'time $(BINDIR)/bench1 -t 8 -l 1 -m 2000'
	@echo ""
	@echo "Run 5:"
	bash -c 'time $(BINDIR)/bench1 -t 16 -l 1 -m 2000'
	@echo ""

bench2: sync
	$(CC) $(CFLAGS) $(SRCDIR)/bench2.c $(BINDIR)/sync.o -o $(BINDIR)/bench2

run-bench2: bench2
	@echo "Run 1:"
	bash -c 'time $(BINDIR)/bench2 -c 1 -s 1 -t 100000000'
	@echo ""
	@echo "Run 2:"
	bash -c 'time $(BINDIR)/bench2 -c 2 -s 1 -t 100000000'
	@echo ""
	@echo "Run 3:"
	bash -c 'time $(BINDIR)/bench2 -c 3 -s 1 -t 100000000'
	@echo ""
	@echo "Run 4:"
	bash -c 'time $(BINDIR)/bench2 -c 4 -s 1 -t 100000000'
	@echo ""
	@echo "Run 5:"
	bash -c 'time $(BINDIR)/bench2 -c 20 -s 2 -t 100000000'
	@echo ""


bench3: sync
	$(CC) $(CFLAGS) $(SRCDIR)/bench3.c $(BINDIR)/sync.o -o $(BINDIR)/bench3

run-bench3: bench3
	@echo "Run 1:"
	bash -c 'time $(BINDIR)/bench3 -t 1 -x 100 -y 50000'
	@echo ""
	@echo "Run 2:"
	bash -c 'time $(BINDIR)/bench3 -t 2 -x 100 -y 50000'
	@echo ""
	@echo "Run 3:"
	bash -c 'time $(BINDIR)/bench3 -t 4 -x 100 -y 50000'
	@echo ""
	@echo "Run 4:"
	bash -c 'time $(BINDIR)/bench3 -t 8 -x 100 -y 50000'
	@echo ""
	@echo "Run 5:"
	bash -c 'time $(BINDIR)/bench3 -t 16 -x 100 -y 50000'
	@echo ""

sync:
	$(CC) $(CFLAGS) -c $(SRCDIR)/sync.c -o $(BINDIR)/sync.o

clean:
	rm -rfv $(BINDIR)/*.o $(BINDIR)/bench*
