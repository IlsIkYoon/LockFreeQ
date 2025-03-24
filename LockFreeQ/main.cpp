#include "LFreeQ.h"
#include "CrashDump.h"
#include <process.h>

#define LOOPCOUNT 500000
#define THREADCOUNT 10

LFreeQ<int> g_stack;
HANDLE hArr[THREADCOUNT];

UINT ThreadFunc(void*)
{

	for (int i = 0; i < LOOPCOUNT; i++)
	{
		g_stack.Enqueue(i);
		g_stack.Dequeue();
	}


	return 0;
}


int main()
{
	DWORD startTime;
	DWORD endTime;
	DWORD result;
	timeBeginPeriod(1);


	procademy::CCrashDump dump;


	for (int i = 0; i < THREADCOUNT; i++)
	{
		hArr[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, NULL, CREATE_SUSPENDED, NULL);
	}
	//여기서 시간 체크 하고

	startTime = timeGetTime();


	for (int i = 0; i < THREADCOUNT; i++)
	{
		if (hArr[i] == nullptr)
			__debugbreak();

		ResumeThread(hArr[i]);
	}

	WaitForMultipleObjects(THREADCOUNT, hArr, true, INFINITE);


	//여기서 시간 체크해서 비교 하면 되겠다
	endTime = timeGetTime();

	result = endTime - startTime;
	printf("result : %d\n", result);


	__debugbreak();

	printf("종료 완료\n");







	return 0;
}