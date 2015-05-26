#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <assert.h>
using namespace std;

void CountPlusWithMemoryMap();
void mmap_demo();

int main()
{
	//mmap_demo();
	//CountPlusWithMemoryMap();

	return 0;
}

void mmap_demo()
{
	//先在当前目录下准备好一个content文件，里面写一些英文
	int fd = open("content", O_RDWR, S_IRUSR | S_IWUSR);
	if( -1 == fd ){
		printf("open shm failed! reason:%s\n", strerror(errno));
		exit(-1);
	}

	//截取fd中的内容前10个字节，可以试试把这句去掉运行看结果
	//note:执行这个函数后，content文件里的内容也会相应的改变，打开content看一下
	ftruncate(fd, 10);
	char *ptr = (char*)mmap(NULL, 10, PROT_READ | PROT_WRITE, O_RDWR, fd, 0);
	if( MAP_FAILED == ptr ){
		printf("mmap failed! reason:%s\n", strerror(errno));
		exit(-1);
	}
	close(fd);

	fputs(ptr, stdout);

	munmap(ptr, 10);
}


/*=============================================================================
 * 功能描述: 父子进程给共享内存区中的一个计数器加1
 *
 * Note:    mmap()适用于具有亲缘关系的进程之间共享内存,因为子进程会继承父进程的
 * 映射关系，所以无论父子进程谁动了文件，另一方都能相应的同步。但如果是在两个
 * 无关进程中，一方修改了文件，另一方是无法进行同步的，所以这种将文件映射到
 * 内存中的方式仅限于具有亲缘关系的进程之间使用。如果需要在无关进程中使用内存
 * 映射，可以先使用shm_open创建一个共享内存对象，然后再用mmap进行映射。
 ==============================================================================*/
void CountPlusWithMemoryMap()
{
	int fd = -1;
	int zero = 0;
	int *ptr = NULL;
	sem_t *mutex = SEM_FAILED;

	//创建文件
	fd = open("./config", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if( -1 == fd ){
		cout<<"open file failed! reason:"<<strerror(errno)<<endl;
		exit(-1);
	}

	//向文件中写入初始数据
	write(fd, &zero, sizeof(int));

	//将文件内容映射到进程空间中
	ptr = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if( MAP_FAILED == ptr ){
		cout<<"memory map failed! reason:"<<strerror(errno)<<endl;
		close(fd);
		exit(-1);
	}
	close(fd);

	//创建命名信号量，由于其至少是内核持续的，所以在各进程间都可共享使用
	mutex = sem_open("/mutex", O_CREAT | O_EXCL, S_IRUSR | S_IRUSR);
	if( SEM_FAILED == mutex ){
		cout<<"create mutex failed! reason:"<<strerror(errno)<<endl;
		munmap(ptr, sizeof(int));
		exit(-1);
	}
	sem_unlink("/mutex");//此处务必unlink mutex，因为其至少是内核持续的，关闭进程后其依然存在

	setbuf(stdout, NULL);
	if( 0 == fork() ){
		for (int i = 0; i < 100; i++){
			usleep(100000);
			sem_wait(mutex);
			printf("child:%d\n", (*ptr)++);
			sem_post(mutex);
		}
		exit(0);
	}

	for (int i = 0; i < 100; i++){
		usleep(100000);
		sem_wait(mutex);
		printf("parent:%d\n", (*ptr)++);
		sem_post(mutex);
	}

	exit(0);
}
