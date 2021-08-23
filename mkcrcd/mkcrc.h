#define BLOCK_SIZE_DEFAULT 512
#define BLOCK_SIZE_MIN 64
#define BLOCK_SIZE_MAX 8192
#define COUNT_DEFAULT 499
#define COUNT_MAX 1999  /* just so we don't accidentally fill filesystem */
#define COUNT_MIN 1

#define MIN_VALID 8 /* min number of valid bytes at start of each buffer -- can be 0 */
#define MIN_BURST 16 /* min burst length, must be big enough to avoid birthday issues */

/* flags */
#define F_VERBOSE 0x1

void mk_err_buf(char *, char *, int);

/* set structure */

struct set_elem {
    unsigned se_hash;   /* for possible efficiency later */
    char se_buf[BLOCK_SIZE_MAX];
};

#define MAX_SET_SIZE (COUNT_MAX+1)
struct error_set {
    int es_size;
    int es_buf_size;
    struct set_elem es_members[MAX_SET_SIZE];
};

extern struct error_set *new_error_set(int, int);
extern int add2set(char *,struct error_set *);
extern void destroy_set(struct error_set *);
