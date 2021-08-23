/*
 * tool to test CRC performance on packets that suffer burst errors similar to those experienced
 * on the Internet
 *
 * designed currently to run in a directory filled with files created by mkcrcd
 *
 * usage: testcrcburst crc-file directory
 *
 * where crc-file is a list of 16-bit CRC polynomials (one per line)
 *
 * NOTE: needs to be upgraded to handle CRCs of arbitrary lengths...
 */

#include <stdio.h>
#include <strings.h>
#include <sys/stat.h>

#include "crc-list.h"
#include "testcrcburst.h"


/*
 *
 */

int init(int argc, char **argv,unsigned int *flags,char **dir, char **crc, char *progname)
{
    int i=0;
    struct stat s;

    argv++;
    while (*argv != 0)
    {
        if (strcmp(*argv,"-v")==0)
        {
            *flags |= F_VERBOSE;
        }
        else
        {
            switch(i++)
            {
                case 0:
                    *dir = *argv;
                    break;

                case 1:
                    *crc = *argv;
                    break;

                default:
                    goto usage;
            }
        }
        argv++;
    }

    if (i != 2)
        goto usage;

    if ((stat(*dir,&s) < 0) || ((s.st_mode &S_IFDIR)==0))
    {
        (void) fprintf(stderr,"%s: error accessing directory %s\n",progname,*dir);
        return(1);
    }
    if ((stat(*crc,&s) < 0) || ((s.st_mode &S_IFREG)==0))
    {
        (void) fprintf(stderr,"%s: error accessing file %s\n",progname,*crc);
        return(1);
    }

    return(0);

usage:
    (void) fprintf(stderr,"usage: %s [-v] directory crc-file\n",progname);
    return(1);
}


/*
 *
 */

int main(int argc, char **argv)
{
    unsigned int flags;
    char *dirname=0;
    char *progname=0;
    char *crcfilename=0;
    int crclen = 0;

    progname = argv[0];

    if (init(argc,argv,&flags,&dirname,&crcfilename,progname) != 0)
        return(1);

    if (loadcrcs(crcfilename,flags,progname) != 0)
        return(1);

    if (scandirectory(dirname,flags,progname) != 0)
        return(1);

    return(0);
}

