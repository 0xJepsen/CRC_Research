/*
 * mkcrcd - makes data files for crc tests
 *
 * usage: mkcrcd srcfile [-v] [-s blocksize] [-c count]
 *
 * reads srcfile in chunks of blocksize bytes [default 512] and for each chunk creates count
 * different versions [default 499] with burst errors
 *
 * for each chunk, the original chunk (as first chunk) and the count versions with errors
 * are written in a file named blk#.blocksize.crcd in the current directory.  Note this
 * naming means only one file can be broken up into the current directory (a good thing for
 * experimental cleanliness) but the same file can be broken up multiple times using different
 * block sizes
 *
 * -v turns on progress reporting on stderr
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include "mkcrcd.h"



/*
 * init - uses getopt, which is ugly but simple
 *
 * because I believe in exiting init with as few possible downstream errors as possible
 * we stat the file to confirm it exists...
 *
 */

int init(int argc, char **argv,char **srcfile, unsigned *flags, int *blocksize, int *count,char *progname)
{
    int opt;
    int index;
    struct stat srcstat;
    char *tp;

    if (argc < 2)
        goto usage;

    *srcfile = argv[1];
    optind = 2;

    while ((opt = getopt(argc,argv,":vs:c:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                *flags |= F_VERBOSE;
                break;
            case 's':
                *blocksize = strtol(optarg, &tp, 0);
                if ((tp == optarg) || (*blocksize < BLOCK_SIZE_MIN) || (*blocksize > BLOCK_SIZE_MAX))
                {
                    (void) fprintf(stderr,"%s: invalid or out of range (%d-%d) blocksize\n",
                        progname,BLOCK_SIZE_MIN,BLOCK_SIZE_MAX);
                    return(1);
                }
                break;
            case 'c':
                *count = strtol(optarg, &tp, 0);
                if ((tp == optarg) || (*count > COUNT_MAX) || (*count < COUNT_MIN))
                {
                    (void) fprintf(stderr,"%s: invalid or out of range (%d-%d) count\n",
                        progname,COUNT_MIN,COUNT_MAX);
                    return(1);
                }
                break;

            default:
                goto usage;
                return(1);
        }
    }

    /* can I stat file and is it a regular file? */
    if ((stat(*srcfile,&srcstat) < 0) || ((srcstat.st_mode & S_IFREG)==0))
    {
        (void) fprintf(stderr,"%s: invalid srcfile %s\n",progname,*srcfile);
        return(1);

    }

    if (*flags & F_VERBOSE)
        (void) fprintf(stderr,"reading %s with blocksize %d count %d\n",
            *srcfile,*blocksize,*count);

    /* set random seed */

    srandom(0);

    return(0);

usage:
    (void) fprintf(stderr,"usage: %s srcfile [-v] [-s blocksize] [-c count]\n",progname);
    return(1);
}

/*
 * create the error file -- note uses a set concept to deal with duplicates
 */
int make_error_file(char *newfile,char valid_buf[BLOCK_SIZE_MAX],int bs,int count, char *progname)
{
    FILE *f;
    struct error_set *es = new_error_set(count+1,bs);
    char err_buf[BLOCK_SIZE_MAX];
    int counter;

    if (es == 0)
    {
        (void) fprintf(stderr,"%s: memory failure\n",progname);
        return(1);
    }

    if ((f = fopen(newfile,"w")) == NULL)
    {
        (void) fprintf(stderr,"%s: could not open file %s\n",progname,newfile);
        return(1);
    }

    /*
     * idea here is just keep trying to add errors to the set -- set confirms each error is new,
     * not a repeat
     *
     * include source block in set to ensure no dupes of source block
     */

    (void) add2set(valid_buf,es);
    if (fwrite(valid_buf,sizeof(valid_buf[0]),bs,f) != bs)
    {
        goto short_file;
    }

    counter = 0;


    while (es->es_size < (count+1))
    {
        mk_err_buf(valid_buf,err_buf,bs);
        assert(valid_buf[0]==err_buf[0]);
        if (add2set(err_buf,es))
        {
            if (fwrite(err_buf,sizeof(err_buf[0]),bs,f) != bs)
                goto short_file;
        }

        /* infinite loop prevention */
        if (counter > (4*count))
        {
            goto short_file;
        }
        counter++;
    }


    destroy_set(es);
    (void) fclose(f);
    return(0);

short_file:
    destroy_set(es);
    (void) fprintf(stderr,"%s: warning, short file %s\n",progname,newfile);
    (void) fclose(f);
    return(1);
}

/*
 * split the file
 */

int split_file(char *src,unsigned flags,int bs, int count, char *progname)
{
    FILE *f;
    char buffer[BLOCK_SIZE_MAX];
    char newfile[256];
    struct stat fstat;
    int total_blocks, blocks_processed, blocks_10;

    /* open source file and get ready to track progress in blocks */

    if ((f= fopen(src,"r")) == NULL)
    {
        (void) fprintf(stderr,"%s: error opening %s\n",progname,src);
        return(1);
    }

    if (stat(src,&fstat) < 0)
    {
        /* this should not happen, as did stat in init, and file is open but... */
        (void) fprintf(stderr,"%s: error checking permissions  %s\n",progname,src);
        return(1);

    }
    total_blocks = fstat.st_size / bs;
    blocks_processed = 0;
    blocks_10 = total_blocks / 10;

    /* while new full sized block */
    while (fread(buffer,sizeof(buffer[0]),bs,f) == bs)
    {
        /* create new file name */
        (void) snprintf(newfile,sizeof(newfile),"blk%06d.%d.crcd",blocks_processed,bs);

        if (make_error_file(newfile,buffer,bs,count,progname) != 0)
            break;

        blocks_processed++;

        if (flags & F_VERBOSE)
        {
            if ((blocks_processed % blocks_10) == 0)
            {
                (void) fprintf(stderr,"%s: %d0%% complete\n",progname,blocks_processed/blocks_10);
            }
        }
    }

    if (!feof(f))
        (void) fprintf(stderr,"%s: warning - error while processing %s\n",progname,src);

    (void) fclose(f);
    return(0);
}


/*
 * really simple main - get args then do the work
 */

int main(int argc, char **argv)
{
    char *srcfile = 0;
    int blocksize = BLOCK_SIZE_DEFAULT;
    int count = COUNT_DEFAULT;
    unsigned flags = 0;
    char *progname = argv[0];   /* for stderr messages */

    if (init(argc,argv,&srcfile,&flags,&blocksize,&count,progname) != 0)
        return(1);

    if (split_file(srcfile,flags,blocksize,count,progname) != 0)
        return(1);

    return(0);
}

