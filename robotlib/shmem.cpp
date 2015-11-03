#include "shmem.h"
#include "CoCycle.h"
using namespace chaos;

extern CoCycle * g_cycle;

Shmem::Shmem(key_t shkey ,long int count)
{
	ptrmem = (char*)0;
	memcount = count;
	mshkey = shkey;
	CreatShareMemory();
}

//创建共享内存
int Shmem::CreatShareMemory( )
{
	if( (shmid = shmget(mshkey ,memcount, 0666|IPC_CREAT|IPC_EXCL)) == -1)
	{
		//共享内存已经存在
		iFlag = 2;
		if( (shmid = shmget(mshkey ,memcount, 0)) == -1)
			return -1;
		
		//GetShareMemory();
	}
	else
	{
		//共享内存不存在,初始化
		iFlag = 1;
/*		char *pch = (char*)GetShareMemory();
		if( pch!=(char*)0 && pch!=(char*)-1 )
		{
			bzero( pch ,memcount );
		}
		else return -1;*/
	}
	return 0;
}

char* Shmem::GetShareMemory()
{
	if( shmid!= -1) {
		ptrmem = (char*)shmat(shmid,0,0) ;
		if(iFlag == 1)		bzero(ptrmem, memcount);
		return ptrmem;
	}
	else return (char*)0;
}

/*char* Shmem::GetShareMemory()
{
	return (char*)ptrmem;
}
*/

int Shmem::FreeShareMemory()
{
	if( ptrmem!=(char*)0 || ptrmem!=(char*)-1 )
		return(shmdt( ptrmem ));

	return 0;
}

Shmem::~Shmem()
{
	//FreeShareMemory();
}
