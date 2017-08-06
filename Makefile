
GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)

CC=gcc -Wall -Wextra -O3
CPP=g++ -Wall -Wextra -O3
GR_COMP=gnuradio-runtime gnuradio-blocks gnuradio-dtv gnuradio-osmosdr
CFLAGS=`pkg-config --cflags $(GR_COMP)` -DVERSION=\"$(GIT_VERSION)\"
LDFLAGS=`pkg-config --libs $(GR_COMP)` -lboost_system

all: dvbs2_tx dvbs2_rate

dvbs2_tx: dvbs2_tx.o app_conf.o
	$(CPP) -o dvbs2_tx dvbs2_tx.o app_conf.o $(LDFLAGS)

dvbs2_tx.o: dvbs2_tx.cpp
	$(CPP) -c $(CFLAGS) dvbs2_tx.cpp

app_conf.o: app_conf.cpp
	$(CPP) -c $(CFLAGS) app_conf.cpp

dvbs2_rate:
	$(CC) -o dvbs2_rate dvbs2_rate.c -lm


.PHONY: clean

clean:
	rm dvbs2_tx dvbs2_rate *.o
