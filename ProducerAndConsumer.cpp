/*===================================================================
 * 功能描述：生产者消费者模型
 * 参考技术：消息队列、线程锁的使用
 * 运行环境：linux
=====================================================================*/

#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <mqueue.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

#define CHK(x) do{\
	if( -1 == x ){\
		cout<<strerror(errno)<<"! error at line:"<<__LINE__<<endl;\
		exit(-1);\
	}\
}while(0);

#define MSGQUE_NAME		"/msgqueue"
#define MAX_MSG			10          //以系统的设定值为准
#define MSG_SIZE		8192        //以系统的设定值为准
#define PRODUCER_NUM	3
#define CONSUMER_NUM	2

unsigned int msg_prio = 0;
bool p_contnu = true;
bool c_contnu = true;
static int data = 0;

mqd_t mid;
pthread_mutex_t mutex;

void* Producer(void* num);
void* Consumer(void* num);

int main()
{
	/* msg queue set */
	int flag = O_RDWR | O_CREAT | O_NONBLOCK;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	struct mq_attr attr;
	attr.mq_flags = flag;
	attr.mq_maxmsg = MAX_MSG;
	attr.mq_msgsize = MSG_SIZE;

	mid = mq_open(MSGQUE_NAME, flag, mode, &attr);
	CHK(mid);

	pthread_mutex_init(&mutex, NULL);

	/* create producer */
	pthread_t producer[PRODUCER_NUM];
	for(int i = 0; i < PRODUCER_NUM; i++){
		if( 0 == pthread_create(&producer[i], NULL, Producer, &i) ){
			cout<<"create producer_"<<i<<" success!\n";
		}
		sleep(1);
	}
	
	/* create consumer */
	pthread_t consumer[CONSUMER_NUM];
	for(int i = 0; i < CONSUMER_NUM; i++){
		if( 0 == pthread_create(&consumer[i], NULL, Consumer, &i) ){
			cout<<"create consumer_"<<i<<" success!\n";
		}
		sleep(1);
	}

	//debug cmd:
	//1. 'q': quit
	//2. 's': show curmsg number at msgqueue
	//3. 'p': stop producer
	//4. 'c': stop consumer
	//5. 'w': print producer and consumers' working state
	char input = 'a';
	while( input != 'q' )
	{
		input = (char)getchar();

		switch (input) {
			case 's': {
				struct mq_attr attr;
				mq_getattr(mid, &attr);
				cout<<"curmsg number = "<<attr.mq_curmsgs<<endl;
				break;
			}
			case 'p': {
				p_contnu = false;
				break;
			}
			case 'c': {
				c_contnu = false;
				break;
			}
			case 'w': {
				cout<<"p_contnu="<<p_contnu<<endl;
				cout<<"c_contnu="<<c_contnu<<endl;
				break;
			}
		}
	}
	p_contnu = false;
	c_contnu = false;

	for(int i = 0; i < PRODUCER_NUM; i++){
		pthread_join(producer[i], NULL);
	}
	for(int i = 0; i < CONSUMER_NUM; i++){
		pthread_join(consumer[i], NULL);
	}

	pthread_mutex_destroy(&mutex);

	CHK( mq_close(mid) );
	CHK( mq_unlink(MSGQUE_NAME) );
	
	return 0;
}

void* Producer(void* num)
{
	int pro_id = *(int*)num;
	char wBuf[32] = {0};

	while(p_contnu)
	{
		sleep(1);

		memset(wBuf, 0, sizeof(wBuf));
		sprintf(wBuf, "%d", data);

		pthread_mutex_lock(&mutex);
		mq_send(mid, wBuf, strlen(wBuf), msg_prio);//forbide blocking here
		if( errno == EAGAIN ){
			errno = 0;//clear errno for fear of interference
			pthread_mutex_unlock(&mutex);
			continue;
		}
		
		//cout<<"Producer_"<<pro_id<<" produce "<<data<<endl;
		data++;
		pthread_mutex_unlock(&mutex);
	}
}

void* Consumer(void* num)
{
	int consumer_id = *(int*)num;
	char rBuf[MSG_SIZE] = {0};

	while(c_contnu)
	{
		usleep(10);

		memset(rBuf, 0, sizeof(rBuf));

		pthread_mutex_lock(&mutex);
		mq_receive(mid, rBuf, sizeof(rBuf), &msg_prio);//forbide blocking here
		if( errno == EAGAIN ){
			errno = 0;//clear errno for fear of interference
			pthread_mutex_unlock(&mutex);
			continue;
		}
		
		cout<<"Consumer_"<<consumer_id<<":"<<atoi(rBuf)<<endl;
		pthread_mutex_unlock(&mutex);
	}
}
