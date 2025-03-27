#pragma comment(lib, "winmm.lib")

#include "LFreeQ.h"
#include "CrashDump.h"
#include <process.h>
#include <conio.h>

#define LOOPCOUNT 5000000
#define THREADCOUNT 10

HANDLE g_event;

LFreeQ<int> g_stack;
HANDLE hArr[THREADCOUNT];

UINT ThreadFunc(void*)
{
	int i = 0;
	while(1)
	{
		g_stack.Enqueue(i++);
		g_stack.Enqueue(i++);
		g_stack.Enqueue(i++);
		g_stack.Dequeue();
		g_stack.Dequeue();
		g_stack.Dequeue();

		DWORD retval = WaitForSingleObject(g_event, 0);
		if (retval == WAIT_OBJECT_0)
		{
			break;
		}
	}


	return 0;
}


int main()
{
	DWORD startTime;
	DWORD endTime;
	DWORD result;
	timeBeginPeriod(1);
	g_event = CreateEvent(NULL, true, false, NULL);

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

	while (1)
	{
		if (_kbhit())
		{
			char c;
			c = _getch();

			if (c == 'q' || c == 's')
			{
				SetEvent(g_event);
				break;
			}
		}
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