
#include <stdio.h>
#include <strings.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/param.h>
#include "crc-list.h"
#include "testcrcburst.h"
#include "../mkcrcd/mkcrcd.h"

/*
 * compare two blocks...
 */

static unsigned hdmasks[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 } ;


void cmp_blocks(char *valid,char *errbuf,int bs,char *filename,int blocknum)
{
    int i,j;
    int hd = 0;
    int bytes_differ = 0;
    unsigned c;

    /*
     * start by computing the hamming distance and number of bytes that differ, in case
     * a crc doesn't catch the error
     */

    for(i=0; i < bs; i++)
    {
        if ((c = (valid[i] ^ errbuf[i])) != 0)
        {
            bytes_differ++;
            for(j=0; j < 8; j++)
            {
                if (hdmasks[j] & c)
                {
                    hd++;
                }
            }

        }
    }

    /* now check all those CRCs */
    do_crcs(errbuf,bs,filename,blocknum,hd,bytes_differ);

    return;
}

/*
 * given a file of data, read it
 */

int readfile(char *dir, char *fname, int blocksize, int *count)
{
    FILE *f;
    char firstbuffer[BLOCK_SIZE_MAX];
    char currentbuffer[BLOCK_SIZE_MAX];
    char filename[2*MAXPATHLEN];        /* double system maximum */
    int blocknum;

    (void) snprintf(filename,sizeof(filename),"%s/%s",dir,fname);

    if ((f = fopen(filename,"r")) == NULL)
    {
        return(1);
    }

    if (fread(firstbuffer,sizeof(firstbuffer[0]),blocksize,f) != blocksize)
        goto error;

    init_crcs(firstbuffer,blocksize); /* get CRC system initialized */


    blocknum=1;
    while (fread(currentbuffer,sizeof(currentbuffer[0]),blocksize,f) == blocksize)
    {
        cmp_blocks(firstbuffer,currentbuffer,blocksize,filename,blocknum);
        blocknum++;
        *count = *count+1;
    }

    if (!feof(f))
        goto error;
    (void) fclose(f);
    return(0);

error:
    (void) fclose(f);
    return(1);
}


/*
 *
 */


int scandirectory(char *dirname, unsigned flags,char *progname)
{
    DIR *d;
    struct dirent *dep;
    int blocksize;
    int blocksize_smallest = BLOCK_SIZE_MAX;
    int blocksize_largest = 0;
    int filecount=0;
    int scannederrors=0;

    (void) printf("Polynomial,\"Initial Remainder\",File,Block,\"Hamming Distance\",\"Bytes Differ\",Comment\n");

    if ((d = opendir(dirname)) == NULL)
    {
        (void) fprintf(stderr,"%s: can't open directory %s\n",progname,dirname);
        return(1);
    }

    /* look for files containing mkcrcd data */

    while ((dep = readdir(d)) != NULL)
    {
        if (sscanf(dep->d_name,"blk%*d.%d.crcd",&blocksize) != 1)
            continue;

        if (blocksize > BLOCK_SIZE_MAX)
        {
            (void) fprintf(stderr,"%s: block size too big (>%d) in file %s/%s\n",progname,
                BLOCK_SIZE_MAX,dirname,dep->d_name);
        }
        else
        {
            if (blocksize > blocksize_largest)
            {
                blocksize_largest = blocksize;
            }
            if (blocksize < blocksize_smallest)
            {
                blocksize_smallest = blocksize;
            }
        }


        if (readfile(dirname,dep->d_name,blocksize,&scannederrors) != 0)
        {
            (void) fprintf(stderr,"%s: problems reading file %s/%s\n",progname,dirname,dep->d_name);
        }
        else
        {
            /* successfully scanned a file with block and its errored versions */
            filecount++;
        }
    }

    (void) printf(",,,,,,%d files\n",filecount);
    (void) printf(",,,,,,%d errored blocks\n",scannederrors);
    (void) printf(",,,,,,smallest block %d bytes\n",blocksize_smallest);
    (void) printf(",,,,,,largest block %d bytes\n",blocksize_largest);

    if (flags & F_VERBOSE)
    {
        (void) fprintf(stderr,"%s: processed %d files\n",progname,filecount);
    }

    (void) closedir(d);
    return(0);

}

