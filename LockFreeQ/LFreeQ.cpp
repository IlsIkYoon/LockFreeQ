#include "LFreeQ.h"


LogData* logArr;
unsigned long long g_SeqNum = 0;



void WriteAllLogData()
{

	printf("Memory Log Save Start!!!\n");
	std::string fileName;
	fileName = __DATE__;
	fileName += "Memory_Log.csv";

	FILE* fpWrite;
	fopen_s(&fpWrite, fileName.c_str(), "a");
	if (fpWrite == 0) __debugbreak();
	fprintf(fpWrite, "SeqNum,FuncType,Address,TID,threadNext,nodeNext,createCount,deleteCount\n");


	for (int i = 0; i < g_SeqNum; i++)
	{

		if ((logArr[i].funcType & DEQUEUE) == DEQUEUE)
		{
			fprintf(fpWrite, "%llu,DEQUEUE, %16llx,%lu,%16llx,%16llx,%lu,%lu\n", logArr[i].seqNum, logArr[i].myAddress,
				logArr[i].TID, logArr[i].threadNext, logArr[i].nodeNext, logArr[i].createCount, logArr[i].deleteCount);
		}
		else
		{
			fprintf(fpWrite, "%llu,ENQUEUE, %16llx,%lu,%16llx,%16llx,%lu,%lu\n", logArr[i].seqNum, logArr[i].myAddress,
				logArr[i].TID, logArr[i].threadNext, logArr[i].nodeNext, logArr[i].createCount, logArr[i].deleteCount);
		}

	}

	fclose(fpWrite);
	printf("Memory Log Save Fin!!!\n");


	return;


}