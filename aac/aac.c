#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "utmplib.c"

#define RECORDSIZE 5120

struct utmp_record{
    short ut_type;                 //type
    char ut_user[UT_NAMESIZE];     // user name
    char ut_line[UT_LINESIZE];    //tty
    int32_t login_sec;           //登陆时间
    int32_t logout_sec;          //注销时间
    double diff;                 //登陆时间与注销时间的时间差
};
struct utmp_record record[RECORDSIZE];
int num_rd = 0;  //整理日志后的记录数量

void show_wtmp(struct utmp*);  //打印wtmp内容
void showtime(long);         //打印时间（按照时间格式）
void show_sorted();
void  show_totaltime();    //打印总连接时间
void show_splittime();
double sec2hour(double );  //秒换算成小时
#define max(a, b) a>b?a:b

int  main(int ac, char* av[]){
    int wtmpfd;
    struct utmp* utbuf=utmp_next();
    int i;
    int hasf = 0;
    char realPath[256];
    char userName[256];
    memcpy(realPath, WTMP_FILE, 256);
    
    // 处理arg的部分
    if(ac>3){
    // 意思是存在有-f的操作符号
      hasf = 1;
      int id = -1;
      for(i=1;i<ac;++i){
          if(strncmp(av[i], "-f", 2)==0){
            if(i!=ac-1)
            {
              memcpy(realPath, av[i+1], 256);
            }
            id = i; 
          }
      }
      if(id==-1){
        perror("wrong instruction!");return -1;
      }
      
      if(id==1){
        memcpy(userName, av[3], 256);
      }
      else{
        memcpy(userName, av[1], 256);
      }
    }
    if(ac==3){
      if(strncmp(av[1], "-f", 2)!=0){
        perror("wrong instruction!");
        return -1;
      }
      memcpy(realPath, av[2], 256);
    }
    
    if(ac==2){
      memcpy(userName, av[1], 256);
    }
    
    //printf("help you name %s\n", realPath);
    
    
    if(utmp_open(realPath) == -1){
        perror(realPath);
        return -1;
    }

    memset(&record[0],0,sizeof(struct utmp_record)*RECORDSIZE);
    while((utbuf=utmp_next()) != ((struct utmp*)NULL))
    {
        // show_wtmp(utbuf);
        if((utbuf->ut_type == USER_PROCESS))
        {

                for(i=0; i < num_rd; i++)
                {
                    if( (strcmp(record[i].ut_user,utbuf->ut_user)==0)&&
                        (record[i].login_sec == utbuf->ut_time))
                    {
                        break;
                    }

                }
                if(i == num_rd)
                {
                    record[num_rd].ut_type = utbuf->ut_type;
                    memcpy(record[num_rd].ut_user,utbuf->ut_user,UT_NAMESIZE);
                    memcpy(record[num_rd].ut_line,utbuf->ut_line,UT_LINESIZE);
                    record[num_rd].login_sec = utbuf->ut_time;
                    num_rd++;
                }
        }
        else if(utbuf->ut_type == DEAD_PROCESS){
            for(i=0; i < num_rd; i++){
                if(record[i].ut_type == USER_PROCESS
                   && strcmp(record[i].ut_line,utbuf->ut_line)==0
                   && utbuf->ut_time > record[i].login_sec)
                    {
                    record[i].logout_sec = utbuf->ut_time;
                    record[i].ut_type = DEAD_PROCESS;
                    record[i].diff = difftime(record[i].logout_sec,record[i].login_sec);//计算时间差
                }
            }
        }
        else if((utbuf->ut_type == RUN_LVL && strncmp(utbuf->ut_user,"SHUTDOWM",8)==0)
                || (utbuf->ut_type == BOOT_TIME && strncmp(utbuf->ut_user,"REBOOT",6)==0)){
            for(i=0; i < num_rd; i++){
                if(record[i].ut_type == USER_PROCESS
                   && utbuf->ut_time > record[i].login_sec ){
                    record[i].logout_sec = utbuf->ut_time;
                    record[i].ut_type = utbuf->ut_type;
                    record[i].diff = difftime(record[i].logout_sec, record[i].login_sec);
                }
            }
        }
    }

    //  show_sorted();
    
    if(ac == 1 || ac == 3){ //不带参数的ac令命
        show_totaltime();
     }
    else if (ac >= 2){
        //..带参数的ac命令，例如：
        show_split_time(userName);
        // show_splittime();
    }
    return 0;

}

/*
 *      show info()
 *                      displays the contents of the utmp struct
 *                      in human readable form1
 *                      * displays nothing if record has no user name
 */
void show_wtmp( struct utmp *utbufp)
{
    //  if ( utbufp->ut_type != USER_PROCESS )
    //    return;

     printf("%-8.8s", utbufp->ut_name);      /* the logname  */
     printf("");                            /* a space      */
     printf("|%-8.8s", utbufp->ut_line);      /* the tty      */
     printf("");                            /* a space      */
     printf("|%d", utbufp->ut_type);
     printf("");
     showtime( utbufp->ut_time );            /* display time */
#ifdef SHOWHOST
     if ( utbufp->ut_host[0] != '\0' )
         printf(" (%s)", utbufp->ut_host);/* the host    */
#endif
     printf("\n");                          /* newline      */
}

void show_totaltime()
{
    int i=0;
    double total =0.0;
    time_t now_t = time(NULL);
    for(i=0; i < num_rd;i++){
        if(record[i].ut_type != USER_PROCESS){
            total += record[i].diff;
        }
        else {
            record[i].diff = difftime(now_t,record[i].login_sec);
            total += record[i].diff;
        }
    }
    printf("   total    %.2f\n",sec2hour(total));
}

int find_id(char (*ut_user)[UT_NAMESIZE], int cur_user_num, char *user){
    //printf("the current user is %s\n", user);
    for(int i=0; i<cur_user_num; ++i){
    
        if(strncmp(ut_user[i],user, UT_NAMESIZE)==0){
            return i;
        }
    }
    return cur_user_num;
}

void sort_time(char (*ut_user)[UT_NAMESIZE], int user_num, double *time_all){
    char tmp[UT_NAMESIZE];
    for(int i=0;i<user_num; ++i){
        for(int j=i+1;j<user_num; ++j){
            if(strncmp(ut_user[i], ut_user[j], UT_NAMESIZE)>0){
                memcpy(&tmp, &ut_user[i], UT_NAMESIZE);
                memcpy(&ut_user[i], &ut_user[j], UT_NAMESIZE);
                memcpy(&ut_user[j], &tmp, UT_NAMESIZE);
                double tp = time_all[i];
                time_all[i] = time_all[j];
                time_all[j] = tp;
                //printf("swap once\n");
            }
        }
    }
}

void show_splittime(){
    int i = 0;
    int num_users = 0;
    double total =0.0;
    double time_all[num_rd];
    memset(time_all, 0, sizeof(double)*num_rd);

    char ut_user[num_rd][UT_NAMESIZE]; 
    //char **ut_user;
    memset(ut_user, 0, sizeof(char) * num_rd * UT_NAMESIZE);
    
    time_t now_t = time(NULL);
    for(i=0; i<num_rd; ++i){
        //printf("This is on the %d and current user num is %d\n", i, num_users);
        int id = find_id(ut_user, num_users, record[i].ut_user);
        //printf("after find id %d\n", i);
        if(record[i].ut_type != USER_PROCESS){
            total += record[i].diff;
        }
        else {
            record[i].diff = difftime(now_t,record[i].login_sec);
            total += record[i].diff;
        }
        memcpy(&ut_user[id], &record[i].ut_user, UT_NAMESIZE);
        time_all[id] += record[i].diff;
        
        num_users = max(num_users, id+1);
        
    }
    sort_time(ut_user, num_users, time_all);
    for(i=0; i<num_users; ++i){
        printf(" %-20.40s%.2f\n",ut_user[i], sec2hour(time_all[i]));
    }
    printf(" %-20.40s%.2f\n", "total",sec2hour(total));
}

void show_split_time(char user[256]){
    int i = 0;
    int num_users = 0;
    double total =0.0;
    double time_all[num_rd];
    memset(time_all, 0, sizeof(double)*num_rd);

    char ut_user[num_rd][UT_NAMESIZE]; 
    //char **ut_user;
    memset(ut_user, 0, sizeof(char) * num_rd * UT_NAMESIZE);
    
    time_t now_t = time(NULL);
    for(i=0; i<num_rd; ++i){
        if(strncmp(user, record[i].ut_user, UT_NAMESIZE)!=0) continue;
        int id = find_id(ut_user, num_users, record[i].ut_user);
        if(record[i].ut_type != USER_PROCESS){
            total += record[i].diff;
        }
        else {
            record[i].diff = difftime(now_t,record[i].login_sec);
            total += record[i].diff;
        }
        memcpy(&ut_user[id], &record[i].ut_user, UT_NAMESIZE);
        time_all[id] += record[i].diff;
        
        num_users = max(num_users, id+1);
        
    }
    printf(" %-20.40s%.2f\n", user, sec2hour(total));
    return;
    /*
    sort_time(ut_user, num_users, time_all);
    for(i=0; i<num_users; ++i){
        printf(" %-20.40s%.2f\n",ut_user[i], sec2hour(time_all[i]));
    }
    printf(" %-20.40s%.2f\n", "total",sec2hour(total));
    */
}



void showtime( long timeval )
/*
 *      displays time in a format fit for human consumption
 *      uses ctime to build a string then picks parts out of it
 *      Note: %12.12s prints a string 12 chars wide and LIMITS
 *      it to 12chars.
 */
{
        char    *cp;                    /* to hold address of time      */

        cp = ctime(&timeval);           /* convert time to string       */
                                        /* string looks like            */
                                        /* Mon Feb  4 00:46:40 EST 1991 */
                                        /* 0123456789012345.            */
        printf("|%15.15s",cp+4 );       /* pick 12 chars from pos 4     */
}
double sec2hour(double secs){
    double hours = (secs/60.0/60.0);
    
    return hours;
}
