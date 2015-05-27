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

    int count = 0;
    while( !stop && (count < 20) ){
        sleep(1);
        sem_wait(&ptr->store_num);
        sem_wait(&ptr->mutex);
        if( ptr->consume_index >= MAX_MSG_NUM ){
            ptr->consume_index = 0;
        }
        count++;
        cout<<ptr->data[ptr->consume_index++]<<endl;
        sem_post(&ptr->mutex);
        sem_post(&ptr->empty_num);
    }

    munmap(ptr, sizeof(struct tShm));

    return 0;
}

void ProcessSigInt(int signo)
{
    cout<<"receive SIGINT\n";
    stop = true;
}
