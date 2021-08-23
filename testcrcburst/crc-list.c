/*
 * all CRC stuff is here
 *
 * the external world knows to loadcrcs from a file, to compute initial crcs for data
 * and then to call do_crcs() on each errored block, but internals otherwise hidden
 *
 * this is for 16-bit CRCs
 */


#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <sys/types.h>

#include "crc-list.h"


struct crc_state {
    unsigned cs_poly; /* for identity purposes */
    uint16_t cs_init_remainder; /* initial remainder */
    uint16_t cs_crc_tab[256];
};


static struct crc_state crc_list[MAXCRCS];
static int numcrcs = 0;

static uint16_t correct_crcs[MAXCRCS];

/*
 * compute the crc table
 */

void compute_crc_tab(struct crc_state *csp, unsigned poly)
{
    int i;
    uint16_t remainder;
    unsigned char bit;

    /* for each element of table */
    for(i=0; i < 256; i++)
    {
        /* dividend followed by zeros */
        remainder = i << 8;     /* 16-bit dependent! */

        /* do division by bit */
        for (bit=8; bit > 0; bit--)
        {
            if (remainder & 0x8000) /* 16-bit dependent */
            {
                remainder = (remainder << 1) ^ (poly & 0xFFFF);
            }
            else
            {
                remainder = (remainder << 1);
            }

        }
        csp->cs_crc_tab[i] = remainder;
    }
}

/*
 * compute given crc
 */

uint16_t do1crc(struct crc_state *csp,char *buf, int blen)
{
    uint16_t remainder;
    unsigned char c;
    int i;

    remainder = csp->cs_init_remainder;

    for(i=0; i < blen; i++)
    {
        c = buf[i] ^ (remainder >> 8);  /* 16-bit dependent*/
        remainder = csp->cs_crc_tab[c] ^ (remainder << 8);
    }
    return(remainder);
}


/*
 * parse a line to read a new crc
 */

int load1crc(struct crc_state *csp, char *buffer)
{
    unsigned poly, remainder;

    if (strlen(buffer) == 0)
        return(1);

    if (sscanf(buffer,"%x,%x",&poly,&remainder) != 2)
        return(1);

    csp->cs_init_remainder = remainder;
    csp->cs_poly = poly;

    compute_crc_tab(csp,poly);
    return(0);

}


/*
 *
 */


int loadcrcs(char *crcfilename,unsigned int flags,char *progname)
{
    FILE *f;
    char buffer[512];   /* wildly longer than required... */

    if ((f = fopen(crcfilename,"r")) == NULL)
    {
        (void) fprintf(stderr,"%s: can't open file %s\n",progname,crcfilename);
        return(1);
    }

    while (fgets(buffer,sizeof(buffer),f) != (char *)0)
    {
        if (numcrcs >= MAXCRCS)
            continue;   /* silently skip excess */

        if (load1crc(&crc_list[numcrcs],buffer) == 0) /* successful parse */
            numcrcs++;
    }

    (void) fclose(f);

    if (numcrcs == 0)
    {
        (void) fprintf(stderr,"%s: no checksums in file %s\n",progname,crcfilename);
        return(1);
    }
    return(0);
}

/*
 * given a correct buffer, compute the CRC for each CRC algorithm
 */

void init_crcs(char *buff, int buflen)
{
    int i;

    for (i=0; i < numcrcs; i++)
        correct_crcs[i] = do1crc(&crc_list[i],buff,buflen);
}

/*
 * OK, now for a busted buffer, see if the CRCs incorrectly match
 */

void do_crcs(char *badbuff, int bblen, char *filename, int blocknum, int hd, int bytes_differ)
{
    int i;

    for(i=0; i < numcrcs; i++)
    {
        if (do1crc(&crc_list[i],badbuff,bblen) == correct_crcs[i])
        {
            /* CRC fails! */
            (void) printf("0x%4.4X,0x%4.4X,%s,%06d,%d,%d,\n",
                crc_list[i].cs_poly, crc_list[i].cs_init_remainder,
                filename, blocknum, hd, bytes_differ);
        }
    }

}

/*
 * basic unit tests
 */

#ifdef UNIT_TEST

char buffer[] = "123456789";

unsigned short ccitt_tab[256]={
0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF,0x1231,0x0210,0x3273,0x2252,0x52B5,0x4294,0x72F7,0x62D6,0x9339,0x8318,0xB37B,0xA35A,0xD3BD,0xC39C,0xF3FF,0xE3DE,0x2462,0x3443,0x0420,0x1401,0x64E6,0x74C7,0x44A4,0x5485,0xA56A,0xB54B,0x8528,0x9509,0xE5EE,0xF5CF,0xC5AC,0xD58D,0x3653,0x2672,0x1611,0x0630,0x76D7,0x66F6,0x5695,0x46B4,0xB75B,0xA77A,0x9719,0x8738,0xF7DF,0xE7FE,0xD79D,0xC7BC,0x48C4,0x58E5,0x6886,0x78A7,0x0840,0x1861,0x2802,0x3823,0xC9CC,0xD9ED,0xE98E,0xF9AF,0x8948,0x9969,0xA90A,0xB92B,0x5AF5,0x4AD4,0x7AB7,0x6A96,0x1A71,0x0A50,0x3A33,0x2A12,0xDBFD,0xCBDC,0xFBBF,0xEB9E,0x9B79,0x8B58,0xBB3B,0xAB1A,0x6CA6,0x7C87,0x4CE4,0x5CC5,0x2C22,0x3C03,0x0C60,0x1C41,0xEDAE,0xFD8F,0xCDEC,0xDDCD,0xAD2A,0xBD0B,0x8D68,0x9D49,0x7E97,0x6EB6,0x5ED5,0x4EF4,0x3E13,0x2E32,0x1E51,0x0E70,0xFF9F,0xEFBE,0xDFDD,0xCFFC,0xBF1B,0xAF3A,0x9F59,0x8F78,0x9188,0x81A9,0xB1CA,0xA1EB,0xD10C,0xC12D,0xF14E,0xE16F,0x1080,0x00A1,0x30C2,0x20E3,0x5004,0x4025,0x7046,0x6067,0x83B9,0x9398,0xA3FB,0xB3DA,0xC33D,0xD31C,0xE37F,0xF35E,0x02B1,0x1290,0x22F3,0x32D2,0x4235,0x5214,0x6277,0x7256,0xB5EA,0xA5CB,0x95A8,0x8589,0xF56E,0xE54F,0xD52C,0xC50D,0x34E2,0x24C3,0x14A0,0x0481,0x7466,0x6447,0x5424,0x4405,0xA7DB,0xB7FA,0x8799,0x97B8,0xE75F,0xF77E,0xC71D,0xD73C,0x26D3,0x36F2,0x0691,0x16B0,0x6657,0x7676,0x4615,0x5634,0xD94C,0xC96D,0xF90E,0xE92F,0x99C8,0x89E9,0xB98A,0xA9AB,0x5844,0x4865,0x7806,0x6827,0x18C0,0x08E1,0x3882,0x28A3,0xCB7D,0xDB5C,0xEB3F,0xFB1E,0x8BF9,0x9BD8,0xABBB,0xBB9A,0x4A75,0x5A54,0x6A37,0x7A16,0x0AF1,0x1AD0,0x2AB3,0x3A92,0xFD2E,0xED0F,0xDD6C,0xCD4D,0xBDAA,0xAD8B,0x9DE8,0x8DC9,0x7C26,0x6C07,0x5C64,0x4C45,0x3CA2,0x2C83,0x1CE0,0x0CC1,0xEF1F,0xFF3E,0xCF5D,0xDF7C,0xAF9B,0xBFBA,0x8FD9,0x9FF8,0x6E17,0x7E36,0x4E55,0x5E74,0x2E93,0x3EB2,0x0ED1,0x1EF0};


int main(int argc, char **argv)
{
    struct crc_state cs;
    unsigned short s;
    int i;

    /* CCITT CRC */
    if (load1crc(&cs,"0x1021,0xFFFF") != 0)
    {
        (void) fprintf(stderr,"load1crc of CCITT failed!\n");
        return(1);
    }

    if (cs.cs_init_remainder != 0xFFFF)
    {
        (void) fprintf(stderr,"CCITT 16 remainder is wrong\n");
        return(1);
    }

    for(i=0; i < 256; i++)
    {
        if (cs.cs_crc_tab[i] != ccitt_tab[i])
        {
            (void) fprintf(stderr,"CCITT 16 tab wrong at position %i\n",i);
            return(1);
        }
    }

    /* -1 to remove null byte -- could do strlen, but heck */
   if ((s=do1crc(&cs,buffer,sizeof(buffer)-1)) != 0x29B1)
    {
        (void) fprintf(stderr,"do1crc of CITTT failed! (0x%4.4X)\n",s);
        return(1);
    }

    /* CRC 16 */
    bzero((void *)&cs,sizeof(cs));
    if (load1crc(&cs,"0x8005,0x0") != 0)
    {
        (void) fprintf(stderr,"load1crc of CRC 16 failed\n");
        return(1);
    }
    if (do1crc(&cs,buffer,sizeof(buffer)-1)  != 0xFEE8)
    {
        (void) fprintf(stderr,"do1crc of CRC 16 failed\n");
        return(1);

    }
    (void) fprintf(stderr,"unit check success\n");
    return(0);
}

#endif /* UNIT_TEST */

