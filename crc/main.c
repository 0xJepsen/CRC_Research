/**********************************************************************
 *
 * Filename:    main.c
 *
 * Description: A simple test program for the CRC implementations.
 *
 * Notes:       To test a different CRC standard, modify crc.h.
 *
 *
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#include <stdio.h>
#include <string.h>

#include "crc.h"

extern crc crcTable[];


void
main(void)
{
        int i;
        unsigned char  test[] = "123456789";


        /*
         * Compute the CRC of the test message, slowly.
         */
        printf("The crcSlow() of \"123456789\" is 0x%X\n", crcSlow(test, strlen(test)));

        /*
         * Compute the CRC of the test message, more efficiently.
         */
        crcInit();
        printf("The crcFast() of \"123456789\" is 0x%X\n", crcFast(test, strlen(test)));

        /* dump table */
        printf("crctab = \n{");
        for(i=0; i < 255; i++)
            printf("0x%4.4X,",crcTable[i]);

        printf("0x%4.4X}\n",crcTable[255]);


}   /* main() */
