/*
 * byte oriented method for generating permutations
 *
 * about 20x faster than bitwise calculations...
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

struct perm {
    uint8_t p_list[128];
    int p_len;
};

#define NUMBITSET 9
struct perm permlist[NUMBITSET] = {
    {{0},1},
    {{0x80,0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1},8},
    {{},28},
    {{},56},
    {{},70},
    {{}, 56},
    {{},28},
    {{}, 8},
    {{0xff},1}
};

/*
 * ugly to use recursion but makes state tracking easy
 *
 * counts all permutations of b numbers of bits set in a field of
 * c characters.
 */


void findperms(int b, int c, long long *count)
{
    int i;
    int j;

    assert(b>=0);
    assert(c>=0);
    assert(b<=NUMBITSET);
    /* termination cases... */
    if (c == 0) /* no bytes */
    {
        /* no bytes, but no bits, success you found a permutation */
        if (b == 0)
        {
            (*count)++;
        }
        return;
    }
    /*
     * got here because there are bytes left but no bits to set
     * that's also a permutation
     */
    if (b == 0)
    {
        (*count)++;
        return;
    }

    /* bleah, must interate through choices */
    for (i=0; i <= b; i++)
    {
        /* for each permutation of a bit given length */
        for(j=0; j < permlist[i].p_len; j++)
        {
            /* remove that many bits from b, and find the rest later in string */
            findperms(b-i,c-1,count);
        }
    }
}

int main(int argc, char **argv)
{
    long long count=0;

    findperms(6, 16, &count);

    printf("num permutations is %llu\n",count);
    return(0);
}
