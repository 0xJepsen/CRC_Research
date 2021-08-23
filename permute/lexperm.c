#include <stdio.h>
#include <assert.h>

#define NUMBITS 6
#define FIELDBITS 128

/*
 *
 */

void printperm(unsigned char *v, int vlen)
{
    int i;

    for(i=0; i < vlen; i++)
    {
        (void) printf("%d ",v[i]);
    }
    (void) printf("\n");
}

unsigned char perm2char(unsigned char *v, int vlen)
{
    unsigned char p=0;
    unsigned char mask=0x1;
    int i;

    assert(vlen == 8);

    for(i=0; i < 8; i++)
    {
        if (v[i] != 0)
        {
            p  = p | mask;
        }
        mask = (mask << 1);
    }
    return(p);
}


/*
 * algorithm L from Knuth p. 319
 *
 * returns 0 if no more permutations
 * returns non-zero if new permutation
 */

int lexperm(unsigned char *v, int vlen)
{
    int j, k, l;
    unsigned char temp;

    for(j = vlen-2; j >= 0; j--)
    {
        if (v[j] < v[j+1])
            break;
    }
    if (j<0) return(0);

    for(l = vlen-1; l > 0; l--)
    {
        if (v[j] < v[l])
            break;
    }
    temp = v[j];
    v[j] = v[l];
    v[l] = temp;

    l = vlen-1;
    k = j+1;
    while (k < l)
    {
        temp = v[k];
        v[k] = v[l];
        v[l] = temp;
        k++;
        l--;
    }
    return(1);
}


/*
 *
 */

unsigned char set[FIELDBITS];

int main(int argc, char **argv)
{
    int i=0;
    int j=0;
    unsigned long long count;

    for(i=0; i < sizeof(set); i++)
        set[i] = 0;

    j = sizeof(set)-1;
    for(i=0; i < NUMBITS; i++,j--)
    {
        set[j]=1;
    }

    count = 1;
    while (lexperm(set,sizeof(set)))
    {
        count++;
    }
    printf("num permutations is %llu\n",count);
    return(0);
}

