
CC=gcc -Wall -Wextra -O3
CPP=g++ -Wall -Wextra -O3
GR_COMP=gnuradio-runtime gnuradio-blocks gnuradio-dtv gnuradio-osmosdr
CFLAGS=`pkg-config --cflags $(GR_COMP)`
LDFLAGS=`pkg-config --libs $(GR_COMP)` -lboost_system

all: dvbs2_tx dvbs2_rate

dvbs2_tx: dvbs2_tx.o
	$(CPP) -o dvbs2_tx dvbs2_tx.o $(LDFLAGS)

dvbs2_tx.o: dvbs2_tx.cpp
	$(CPP) -c $(CFLAGS) dvbs2_tx.cpp

dvbs2_rate:
	$(CC) -o dvbs2_rate dvbs2_rate.c -lm


.PHONY: clean

clean:
	rm dvbs2_tx dvbs2_rate *.o
