/*=====================================================================
 * 功能描述：生产者-消费者模型实现
 * 技术综合：线程+信号+信号量+Posix命名消息队列
 * 运行环境：linux
======================================================================*/

#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <mqueue.h>

using namespace std;

#define MSGQUEUE_NAME       "/msgqueue"
#define MAX_MSG             10                  //参照系统设置[详见 man mq_overview]
#define MSG_SIZE            8192                //参照系统设置[详见 man mq_overview]
#define PRODUCER_NUM        5
#define CONSUMER_NUM        10
#define STORENUM_NAME       "sem_storenum"
#define EMPTYNUM_NAME       "/sem_emptynum"
#define MUTEX_NAME          "/sem_mutex"

void* Producer(void *id);
void* Consumer(void *id);
void  CloseAndUnlink();
void  Stop(int signo);

long int data = 0;

bool producing = true;
bool consuming = true;

mqd_t mid = -1;

sem_t *sem_storenum = SEM_FAILED;
sem_t *sem_emptynum = SEM_FAILED;
sem_t *sem_mutex    = SEM_FAILED;

int main()
{
    signal(SIGINT, Stop);
    signal(SIGSEGV, Stop);

    //msgqueue set
    int mqoflag = O_CREAT | O_RDWR | O_NONBLOCK | O_EXCL;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    struct mq_attr attr;
    attr.mq_flags = mqoflag;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_SIZE;

    //create msg queue
    mid = mq_open(MSGQUEUE_NAME, mqoflag, mode, &attr);
    if( -1 == mid ){
        cout<<"create msg queue failed! reason:"<<strerror(errno)<<endl;
        return -1;
    }

    //create sempaphore
    int semoflag = O_CREAT | O_RDWR | O_EXCL;
    sem_storenum = sem_open(STORENUM_NAME, semoflag, mode, 0);
    sem_emptynum = sem_open(EMPTYNUM_NAME, semoflag, mode, MAX_MSG);
    sem_mutex    = sem_open(MUTEX_NAME, semoflag, mode, 1);
    if( (sem_storenum == SEM_FAILED) || (sem_emptynum == SEM_FAILED) || (sem_mutex == SEM_FAILED) ){
        CloseAndUnlink();
        cout<<"open semaphore failed! reason:"<<strerror(errno)<<endl;
        return -1;
    }

    pthread_t producer[PRODUCER_NUM];
    pthread_t consumer[CONSUMER_NUM];

    for(int i = 0; i < CONSUMER_NUM; i++){
        pthread_create(&consumer[i], NULL, Consumer, &i);
        pthread_detach(consumer[i]);
        usleep(10000);
    }

    for(int i = 0; i < PRODUCER_NUM; i++){
        int tmp = i;
        pthread_create(&producer[i], NULL, Producer, &i);
        pthread_detach(producer[i]);
        usleep(10000);
    }

    int input = 'a';
    while( input != 'q' ){
        input = (char)getchar();

        switch(input){
            case 'w':{
                cout<<"Producing="<<producing<<endl;
                cout<<"Consuming="<<consuming<<endl;
                break;
            }
            case 'p':{
                producing  = false;
                break;
            }
            case 'c':{
                consuming = false;
                break;
            }
            case 'r':{
                producing = true;
                consuming = true;
                break;
            }
            case 's':{
                struct mq_attr attr_tmp;
                mq_getattr(mid, &attr_tmp);
                cout<<"CurMsgNum="<<attr_tmp.mq_curmsgs<<endl;
                break;
            }
        }
    }

    CloseAndUnlink();

    return 0;
}


#define CHK_TIMEOUT(cid) do{\
    if( errno == ETIMEDOUT ){\
        printf("Consumer_%d consumes timeout!\n", cid);\
        CONSUMER_QUIT(cid);\
    }\
}while(0)

#define CONSUMER_QUIT(cid) do{\
    printf("Consumer_%d terminate!\n", cid);\
    pthread_exit(NULL);\
}while(0)

void* Consumer(void *id)
{
    int cid = *(int*)id;
    printf("Consumer_%d is running!\n", cid);

    time_t rawtime;
    struct tm *tminfo;
    struct timespec abs_timeout;

    while(consuming){
        usleep(100000);

        time(&rawtime);
        abs_timeout.tv_sec = rawtime+10;
        abs_timeout.tv_nsec = 0;

        sem_timedwait(sem_storenum, &abs_timeout);//当10s内生产者不供给的话，就结束阻塞，退出线程
        CHK_TIMEOUT(cid);
        sem_timedwait(sem_mutex, &abs_timeout);//当10s内不能获取二元互斥量，则结束阻塞，退出线程
        CHK_TIMEOUT(cid);

        char rBuf[MSG_SIZE] = {0};  
        if(  -1 == mq_receive(mid, rBuf, MSG_SIZE, NULL) ){
            cout<<"receive data failed! reason:"<<strerror(errno)<<endl;
            pthread_exit(NULL);
        }
        printf("Consumer_%d consumes:%s\n", cid, rBuf);

        sem_post(sem_mutex);
        sem_post(sem_emptynum);
    }

    CONSUMER_QUIT(cid);
}


#define PRODUCER_QUIT(pid) do{\
    printf("Producer_%d terminate!\n", pid);\
    pthread_exit(NULL);\
}while(0)

void* Producer(void *id)
{
    int pid = *(int*)id;
    printf("Producer_%d is running!\n", pid);

    while(producing){
        usleep(100000);
        
        //生产者作为数据的提供者，这里不提供超时退出线程的功能
        sem_wait(sem_emptynum);
        sem_wait(sem_mutex);

        char wBuf[8] = {0};
        data++;
        sprintf(wBuf, "%ld", data);
        if( -1 == mq_send(mid, wBuf, strlen(wBuf), 0) ){
            cout<<"send data failed! reason:"<<strerror(errno)<<endl;
            pthread_exit(NULL);
        }
        //printf("Producer_%d produces:%s\n", pid, wBuf);

        sem_post(sem_mutex);
        sem_post(sem_storenum);
    }

    PRODUCER_QUIT(pid);
}

void CloseAndUnlink()
{
    if( -1 != mid ){
        mq_close(mid);
        mq_unlink(MSGQUEUE_NAME);
    }

    if( SEM_FAILED != sem_storenum ){
        sem_close(sem_storenum);
        sem_unlink(STORENUM_NAME);
    }

    if( SEM_FAILED != sem_emptynum ){
        sem_close(sem_storenum);
        sem_unlink(EMPTYNUM_NAME);
    }

    if( SEM_FAILED != sem_mutex ){
        sem_close(sem_mutex);
        sem_unlink(MUTEX_NAME);
    }
}

void Stop(int signo)
{
    printf("receive signal:%d!\n", signo);
    CloseAndUnlink();
    exit(0);
}
