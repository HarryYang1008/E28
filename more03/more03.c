/*  more03.c  - version 0.2 of more
 *	read and print 24 lines then pause for a few special commands
 *	v02: reads user control cmds from /dev/tty
 */
#include "termfuncs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>

#define  PAGELEN    24
#define  ERROR        1
#define  SUCCESS    0
#define  has_more_data(x)   (!feof(x))
#define    CTL_DEV    "/dev/tty"        /* source of control commands	*/

int do_more(FILE *);

int how_much_more(FILE *);

void print_one_line(FILE *);

int rows_cols[2];


void handleSigWinCh(int signo) {
    (void) (signo);
    get_term_size(rows_cols);


}


int main(int ac, char *av[]) {
    setbuf(stdout, NULL);
    signal(SIGWINCH, handleSigWinCh);
    get_term_size(rows_cols);


    FILE *fp;
    int result = SUCCESS;

    if (ac == 1)
        result = do_more(stdin);
    else
        while (result == SUCCESS && --ac)
            if ((fp = fopen(*++av, "r")) != NULL) {
                result = do_more(fp);
                fclose(fp);
            } else
                result = ERROR;
    return result;
}


int config_tty(FILE *fp, struct termios *prevTermios) {
    struct termios t;
    int fd = fileno(fp);
    if (tcgetattr(fd, &t) == -1) {
        fprintf(stderr, "can not get attr raw mode\n");
        return -1;
    }

    if (prevTermios != NULL)
        *prevTermios = t;

    t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);

    t.c_iflag &= ~(BRKINT | IGNBRK | IGNCR | INLCR |
                   INPCK | ISTRIP | IXON | PARMRK);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &t) == -1)
        return -1;
    return 0;
}

/*  do_more -- show a page of text, then call how_much_more() for instructions
 *      args: FILE * opened to text to display
 *      rets: SUCCESS if ok, ERROR if not
 */
int do_more(FILE *fp) {
    int num_of_lines = 0;
    int reply;
    struct termios userTermios;

    FILE *fp_tty = fopen(CTL_DEV, "r");       /* NEW: cmd stream   */
    if (fp_tty == NULL)               /* if open fails     */
        exit(1);                           /* no use in running */
    if (config_tty(fp_tty, &userTermios) == -1) {
        fprintf(stderr, "error in setting termial to raw mode\n");
        exit(1);
    }
    while (has_more_data(fp)) {
        if (num_of_lines == rows_cols[0] - 1) {    /* full screen?	*/
            reply = how_much_more(fp_tty);    /* NEW: pass FILE *  */
            if (reply == 0)        /*    n: done   */
                break;
            num_of_lines -= reply;        /* reset count	*/
        }
        print_one_line(fp);

        num_of_lines++;                /* count it	*/
    }
    if (tcsetattr(fileno(fp_tty), TCSAFLUSH, &userTermios) == -1)
        exit(1);
    fclose(fp_tty);
    return 0;
}


/*  print_one_line(fp) -- copy data from input to stdout until \n or EOF */
void print_one_line(FILE *fp) {
    int c;

    while ((c = getc(fp)) != EOF && c != '\n')
        putchar(c);
    putchar('\n');
}

/*  how_much_more -- ask user how much more to show
 *      args: none
 *      rets: number of additional lines to show: 0 => all done
 *	note: space => screenful, 'q' => quit, '\n' => one line
 */
int how_much_more(FILE *fp) {
    int c;

    while ((c = getc(fp)) != EOF)        /* get user input	*/
    {
        if (c == 'q')            /* q -> N		*/
            return 0;
        if (c == ' ')            /* ' ' => next page	*/
            return PAGELEN;        /* how many to show	*/
        if (c == '\n')        /* Enter key => 1 line	*/
            return 1;
    }
    return 0;
}
