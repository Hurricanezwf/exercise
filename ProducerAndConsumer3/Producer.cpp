#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <queue>
#include <pthread.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
using namespace std;

#define MAX_MSG_NUM     10
#define SHM_NAME        "/shm"

struct tShm
{
    unsigned int value;
    unsigned int produce_index;
    unsigned int consume_index;
    unsigned int data[MAX_MSG_NUM];
    sem_t store_num;
    sem_t empty_num;
    sem_t mutex;
};

void ProcessSigInt(int signo);

static bool stop = false;
int main()
{
    signal(SIGINT, ProcessSigInt);
    setbuf(stdout, NULL);//设置标准输出无缓冲

    int shm_fd = -1;
    struct tShm *ptr = NULL;

    //create share memory
    shm_unlink(SHM_NAME);
    shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if( -1 == shm_fd ){
        printf("create share memory failed! reason:%s\n", strerror(errno));
        exit(-1);
    }
    ftruncate(shm_fd, sizeof(struct tShm));

    //do memory mapping
    ptr = (struct tShm*)mmap(NULL, sizeof(struct tShm), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if( ptr == MAP_FAILED ){
       printf("do memory mapping faild! reason:%s\n", strerror(errno));
       shm_unlink(SHM_NAME);
       exit(-1);
    }

    //init semaphore
    sem_init(&ptr->store_num, 1, 0);
    sem_init(&ptr->empty_num, 1, MAX_MSG_NUM);
    sem_init(&ptr->mutex, 1, 1);

    //init struct tShm
    ptr->value = 0;
    ptr->produce_index = 0;
    ptr->consume_index = 0;
    memset(ptr->data, 0, sizeof(ptr->data));

    //start produce
    while(!stop){
        usleep(10000);
        sem_wait(&ptr->empty_num);
        sem_wait(&ptr->mutex);
        if( ptr->produce_index >= MAX_MSG_NUM ){
            ptr->produce_index = 0;
        }
        ptr->data[ptr->produce_index++] = ++ptr->value;
        cout<<"produce:"<<ptr->value<<endl;
        sem_post(&ptr->mutex);
        sem_post(&ptr->store_num);
    }

    sem_destroy(&ptr->store_num);
    sem_destroy(&ptr->empty_num);
    sem_destroy(&ptr->mutex);

    munmap(ptr, sizeof(struct tShm));
    shm_unlink(SHM_NAME);

    return 0;
}

void ProcessSigInt(int signo)
{
    cout<<"receive SIGINT\n";
    stop = true;
    shm_unlink(SHM_NAME);
}
