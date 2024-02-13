/* dumputmp.c 
 *
 *	purpose:	display contents of wtmp or utmp in readable form
 *	args:		none for default (/etc/utmp) or a filename
 *	action:		open the file and read it struct by struct,
 *			displaying all members of struct in nice columns
 *	history:	Feb 06 2021: changed ctime to strftime
 *	history:	Feb 15 1996: added buffering using utmplib
 *	compiling:	to compile this, use
 *			gcc dumputmp.c utmplib.c -o dumputmp
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<utmp.h>
#include	<time.h>
#define	DATE_FMT	"%b %e %H:%M"		/* text format	*/

main(int ac,char **av)
{
	if ( ac == 1 )
		dumpfile( UTMP_FILE );
	else
		dumpfile( av[1] );
}

dumpfile( char *fn )
/*
 * open file and dump records
 */
{
	struct utmp	*utp,		/* ptr to struct	*/
			*utmp_next();	/* declare its type	*/

	if ( utmp_open( fn ) == -1 )	/* open file		*/
	{
		perror( fn );
		return ;
	}

	/* loop, reading records and showing them */

	while( utp = utmp_next() )
		show_utrec( utp );
	utmp_close();
}
	
show_utrec( struct utmp *rp )
{
	char	*typename();
	void	showtime(time_t,char *);

	printf("%-8.8s ", rp->ut_user );
	printf("%-4.4s ", rp->ut_id   );
	printf("%-8.8s ", rp->ut_line );
	printf("%6d ", rp->ut_pid );
	printf("%4d %-14.14s ", rp->ut_type , typename(rp->ut_type) );
	printf("%12d ", (long)rp->ut_time );
	showtime(rp->ut_time, DATE_FMT);
	printf(" ");
	// printf("[%15.15s]", 4+ctime(&rp->ut_time));
	// printf("%12.12s", 4+ctime(&rp->ut_time));
	printf("%s", rp->ut_host );
	putchar('\n');
}

char *uttypes[] = {  	"EMPTY", "RUN_LVL", "BOOT_TIME", "OLD_TIME", 
			"NEW_TIME", "INIT_PROCESS", "LOGIN_PROCESS", 
			"USER_PROCESS", "DEAD_PROCESS", "ACCOUNTING"
	};

char *
typename( int typenum )
{
	return uttypes[typenum];
}

#define	MAXDATELEN	100

void showtime( time_t timeval , char *fmt )
/*
 * displays time in a format fit for human consumption.
 * Uses localtime to convert the timeval into a struct of elements
 * (see localtime(3)) and uses strftime to format the data
 */
{
	char	result[MAXDATELEN];

	struct tm *tp = localtime(&timeval);		/* convert time	*/
	strftime(result, MAXDATELEN, fmt, tp);		/* format it	*/
	fputs(result, stdout);
}

	
