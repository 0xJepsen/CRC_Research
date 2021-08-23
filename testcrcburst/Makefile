CFLAGS=-Ofast

testcrcburst: testcrcburst.o scandirectory.o crc-list.o

testcrcburst.o: testcrcburst.c testcrcburst.h crc-list.h

scandirectory.o: testcrcburst.h scandirectory.c crc-list.h

crc-list.o: crc-list.h crc-list.c

unit: unitcrc

unitcrc: crc-list.c crc-list.h
        cc -DUNIT_TEST -o unitcrc crc-list.c
