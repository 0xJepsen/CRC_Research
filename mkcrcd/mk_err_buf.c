/*
 * create an error buffer from a valid buffer
 *
 * ensure you don't accidentally create a copy of valid buffer
 */

#include <stdlib.h>
#include <strings.h>
#include <assert.h>


#include "mkcrcd.h"

/*
 * create a copy of valid, with a random burst error into ebuf
 */


void mk_err_buf(char *valid, char *ebuf, int bsize)
{
    int i;
    int offset; /* where burst starts */
    int zero_or_rand; /* are we setting zeros or random values? */


    do
    {
        /* copy entire valid into ebuf -- then overwrite portions of ebuf */

        bcopy((void *)valid,(void *)ebuf,bsize);

        /* offset is preceeded by at least some valid data and burst is of minimum length */
        offset = random() % (bsize - (MIN_BURST+MIN_VALID)) ;
        offset += MIN_VALID;

        /* about 25% of errors are all zeros... */
        zero_or_rand = ((random() % 4) == 0);

        assert(offset<bsize);

        for(i=offset; i < bsize; i++)
        {
            ebuf[i] = (zero_or_rand ? 0x0 : random() & 0xFF);
        }
    }
    while (bcmp(valid,ebuf,bsize) == 0);

    assert(valid[0]==ebuf[0]);
}
