

/* crc state table */

#define MAXCRCS 500

extern int loadcrcs(char *,unsigned int ,char *);
extern void do_crcs(char *, int , char *, int , int , int );
extern void init_crcs(char *,int);
