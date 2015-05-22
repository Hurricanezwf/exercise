#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <mqueue.h>
using namespace std;

void handleChildExit(int signo);
int Pipe();
void Popen_demo();
void FIFO_demo();
void posix_msg_queue_demo();

int main()
{
        //Pipe();
        //Popen_demo();
        //FIFO_demo();
        //posix_msg_queue_demo();
        return 0;
}


//pipe
int Pipe()
{
        signal(SIGCHLD, handleChildExit);
        
        int fd[2];
        if( 0 != pipe(fd) )
        {
                cout<<strerror(errno)<<endl;
                return -1;
        }

        if( 0 == fork() )//child process
        {
                close(fd[1]);//close write fd

                int value = 0;
                while(true)
                {
                        read(fd[0], &value, 1);
                        cout<<"child receive:"<<value<<endl;
                        if( value== 9)
                        {
                                break;
                        }
                }

                close(fd[0]);
                exit(1);
        }

        close(fd[0]);

        int val = 0;
        while(val < 10)
        {
                write(fd[1], &val, 1);
                val++;

                sleep(1);
        }

        close(fd[1]);
        wait(NULL);
}

//popen usage demo
void Popen_demo()
{
        const char *cmd = "cat ./IPC.cpp";
        FILE *file = popen(cmd, "r");
        assert( file != NULL);

        char rbuf[64];
        while( NULL != fgets(rbuf, 64, file))
        {
                fputs(rbuf, stdout);
        }
        cout<<endl;

        pclose(file);
}

//FIFO usage demo
void FIFO_demo()
{
        pthread_mutex_t mutex;
        pthread_mutex_init(&mutex, NULL);

        signal(SIGCHLD, handleChildExit);

        mode_t mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);//FIFO mode
        int fd = mkfifo("./FIFO", mode);
        if( (-1 == fd) && (errno != EEXIST) ){
                cout<<strerror(errno)<<endl;
                return;
        }

        if ( 0 == fork() ){      //child process 1 for write
                cout<<"create process1 OK!  pid="<<getpid()<<endl;

                FILE *file = fopen("./FIFO", "a");
                assert( NULL != file );
                
                int count = 0;int val = 1;
                while (count < 100000){
                        pthread_mutex_lock(&mutex);
                        fwrite(&val, sizeof(int), 1, file);
                        pthread_mutex_unlock(&mutex);

                        count++;
                        usleep(100);
                }

                fclose(file);
                exit(1);
        }

        if( 0 == fork() ){      //child process 2 for read
                cout<<"create process2 OK!  pid="<<getpid()<<endl;

                FILE *file = fopen("./FIFO", "r");
                assert( NULL != file );

                int count = 0;
                int value = -1;
                while(true){
                        pthread_mutex_lock(&mutex);
                        if( 0 != fread( &value, sizeof(int), 1, file) ){
                                cout<<"process2 rcv:"<<value<<endl;
                                count++;
                        }
                        pthread_mutex_unlock(&mutex);

                        if( count >= 5 ){
                                break;
                        }

                        usleep(100);
                }

                fclose(file);
                exit(1);
        }

        char input = 'a';
        do{
                input = (char)getchar();
        }while( input != 'q');

        pthread_mutex_destroy(&mutex);
        unlink("./FIFO");//delete FIFO

        return;
}

//Posix Message Queue Demo
//posix msg queue always return the one with the highest priority from the queue firstly
void posix_msg_queue_demo()
{
        signal(SIGCHLD, handleChildExit);

        pthread_mutex_t mutex;
        pthread_mutex_init(&mutex, NULL);

        const char *mq_name = "/mqname";
        mode_t mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        int oflag = (O_CREAT | O_RDWR);

        //the mq_maxmsg and mq_msgsize must be set
        //for more detail, please man 'mq_overview'
        struct mq_attr attr;
        attr.mq_flags = oflag;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = 8192;

        mqd_t mid = mq_open(mq_name, oflag, mode, &attr);//the name must begin with '/'
        if( -1 == mid ){
                cout<<strerror(errno)<<endl;
                return;
        }

        if( 0 == fork() ){      //process 1 for writing
                char wbuf[10] = "";
                int value = 0;
                unsigned int pri = 1;
                while( value < 5){
                        memset(wbuf, 0, sizeof(wbuf));
                        sprintf(wbuf, "send_%d", value);
                        pthread_mutex_lock(&mutex);
                        if( -1 == mq_send(mid, wbuf, strlen(wbuf), ++pri) ){
                                cout<<"err when writing:"<<strerror(errno)<<endl;
                                break;
                        }
                        pthread_mutex_unlock(&mutex);
                        value++;

                        usleep(100);
                }
                exit(1);
        }

        if( 0 == fork() ){      //process 2 for reading
                sleep(2);
                char rbuf[8192] = "";
                int value = 0;
                unsigned int pri = 0;
                while( value < 5 ){
                        memset(rbuf, 0, sizeof(rbuf));
                        pthread_mutex_lock(&mutex);
                        if( -1 == mq_receive(mid, rbuf, 8192, &pri) ){  //*** here msgsize must be equal to  "attr.mq_msgsize" ***//
                                cout<<"err when reading:"<<strerror(errno)<<endl;
                                break;
                        }
                        pthread_mutex_unlock(&mutex);
                        cout<<rbuf<<" pri="<<pri<<endl;
                        value++;

                        usleep(200);
                }
                exit(1);
        }

        char input = 'a';
        do{
                input = (char)getchar();
        }while( input != 'q' );

        pthread_mutex_destroy(&mutex);
        mq_close(mid);
        mq_unlink(mq_name);
        
        return;
}

void handleChildExit(int signo)
{
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if( -1 == pid ){
                cout<<strerror(errno)<<endl;
        }

        cout<<"child terminate at handleChildExit! pid="<<pid<<endl;
}
