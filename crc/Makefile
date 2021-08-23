
all: crc16test ccitttest

crc16test: crc.c main.c crc.h
        cc -DCRC16 -o crc16test crc.c main.c

ccitttest: crc.c main.c crc.h
        cc -DCRC_CCITT -o ccitttest crc.c main.c

