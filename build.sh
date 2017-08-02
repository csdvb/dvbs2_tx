g++ -Wall -Wextra -o dvbs2_tx dvbs2_tx.cpp \
    `pkg-config --cflags --libs gnuradio-runtime gnuradio-blocks gnuradio-dtv gnuradio-osmosdr` \
    -lboost_system
