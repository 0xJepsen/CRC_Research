/*
 * set abstraction
 */

#include <stdlib.h>
#include <strings.h>
#include "mkcrcd.h"


/*
 * create a set - actually don't use set -- can optimze that letter
 */

struct error_set *new_error_set(int count, int buffer_size)
{
    struct error_set *esp;

    if ((esp = (struct error_set *)malloc(sizeof(struct error_set))) == 0)
        return(esp);

    esp->es_size = 0;
    esp->es_buf_size = buffer_size;
    return(esp);
}

/*
 *
 */

void destroy_set(struct error_set *esp)
{
    free((void *)esp);
}


/*
 *
 */

int add2set(char *buf,struct error_set *esp)
{
    int i;

    if (esp->es_size == MAX_SET_SIZE)
        return(0);

    for(i=0; i < esp->es_size; i++)
    {
        if (bcmp(esp->es_members[i].se_buf,buf,esp->es_buf_size) == 0)
            return(0);
    }

    bzero((void *)&(esp->es_members[esp->es_size]),sizeof(struct set_elem));
    bcopy((void *)buf,(void *)esp->es_members[esp->es_size].se_buf,esp->es_buf_size);

    esp->es_size++;
    return(1);
}
