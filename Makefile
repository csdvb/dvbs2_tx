
CC=g++ -Wall -Wextra -O3
GR_COMP=gnuradio-runtime gnuradio-blocks gnuradio-dtv gnuradio-osmosdr
CFLAGS=`pkg-config --cflags $(GR_COMP)`
LDFLAGS=`pkg-config --libs $(GR_COMP)` -lboost_system


dvbs2_tx: dvbs2_tx.o
	$(CC) -o dvbs2_tx dvbs2_tx.o $(LDFLAGS)

dvbs2_tx.o: dvbs2_tx.cpp
	$(CC) -c $(CFLAGS) dvbs2_tx.cpp


.PHONY: clean

clean:
	rm dvbs2_tx *.o
