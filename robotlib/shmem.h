#ifndef _SHMEM_H
#define _SHMEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>

class Shmem
{
public:
	long int memcount;
	int shmid;
	char* ptrmem;
	int iFlag; //如果是初始化的共享内存为1，其他值为其他情况
public:
	int FreeShareMemory();
	Shmem(key_t shkey ,long int count);
	char *GetShareMemory();
	~Shmem();
private:
	Shmem();
	int CreatShareMemory();
	key_t mshkey;
};

#endif
