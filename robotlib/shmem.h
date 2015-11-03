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
	int iFlag; //����ǳ�ʼ���Ĺ����ڴ�Ϊ1������ֵΪ�������
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
