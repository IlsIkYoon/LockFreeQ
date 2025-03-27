#pragma once
#include <windows.h>
#include <iostream>
#include "LMemoryPool.h"

#define MASKING_VALUE17BIT 0x00007fffffffffff
//#define LOG_MAXNUM 5000000000
#define ENQUEUE 0x00
#define DEQUEUE 0x01
#define BIT_64 64
#define UNUSED_BIT 17
#define BIT_OR_VALUE (BIT_64 - UNUSED_BIT)

#define NEWNEXT_VALUE 0x0000000000000001

//#define __LOGDEBUG__

#ifdef __LOGDEBUG__
extern unsigned long long logMaxNum;
extern unsigned long long g_SeqNum;
extern unsigned long long g_CASERROR_SeqNum;
extern unsigned long long g_TailDefer_SeqNum;

struct LogData
{
	char funcType;
	DWORD TID;

	unsigned long long seqNum;

	ULONG_PTR firstCasParam2;
	ULONG_PTR firstCasParam3;
	ULONG_PTR SecondCasParam2;
	ULONG_PTR SecondCasParam3;

	ULONG_PTR NewNodeAddress;
	ULONG_PTR DeleteNodeAddress;

	bool CAS1Success;
	bool CAS2Success;
};

extern LogData* logArr;
extern ULONG64* CASERROR_logArr;
extern LogData* TailDefer_logArr;
#endif

template <typename T>
class LFreeQ
{
	struct Node
	{
		T _data;
		Node* _next;

	};

	Node* _head;
	Node* _tail;
	LMemoryPool<T> _mPool;
	unsigned long long _size;
	unsigned long long _bitCount;

public:

	LFreeQ()
	{
		//_head = (Node*)_mPool.Alloc();
		_head = new Node;
		_head->_next = nullptr;
		_head->_data = 0;
		_tail = _head;
		_size = 0;
		_bitCount = 0;

#ifdef __LOGDEBUG__
		logArr = (LogData*)malloc(sizeof(LogData) * logMaxNum);
		CASERROR_logArr = (ULONG64*)malloc(sizeof(ULONG64) * (logMaxNum));
		TailDefer_logArr = (LogData*)malloc(sizeof(LogData) * (logMaxNum));
#endif
	}




	void Enqueue(T pData)
	{
		Node* localTail;
		Node* exchangeNode;
		Node* localTailAddress;
		Node* nextNode;
		Node* newNode;
		unsigned long long localSeqNum;
		unsigned long long localBitCount;
		DWORD myTID;

		bool CAS1Success;
		bool CAS2Success;


		localBitCount = InterlockedIncrement(&_bitCount);
		myTID = GetCurrentThreadId();

		newNode = (Node*)_mPool.Alloc();
		//newNode = new Node;
		newNode->_data = pData;
		newNode->_next = nullptr;

		exchangeNode = (Node*)((ULONG_PTR)newNode | (localBitCount << BIT_OR_VALUE)); //상위 17비트는 0일 거라는 가정 하에 진행



		while (1)
		{
			CAS1Success = false;
			CAS2Success = false;
			localTail = _tail;
			Node* localHead = _head;
			localTailAddress = (Node*)((ULONG_PTR)localTail & MASKING_VALUE17BIT);

			if (localTailAddress == newNode)
				__debugbreak();

			nextNode = localTailAddress->_next;

			if (nextNode != nullptr)
			{

				if (InterlockedCompareExchangePointer((PVOID*)&_tail, localTailAddress->_next, localTail) == localTail)
				{
#ifdef __LOGDEBUG__
					ULONG64 localTailDeferSeqNum = InterlockedIncrement(&g_TailDefer_SeqNum);
					TailDefer_logArr[localTailDeferSeqNum].CAS1Success = true;
					TailDefer_logArr[localTailDeferSeqNum].funcType = ENQUEUE;
					TailDefer_logArr[localTailDeferSeqNum].firstCasParam2 = (ULONG_PTR)localTailAddress->_next;
					TailDefer_logArr[localTailDeferSeqNum].firstCasParam3 = (ULONG_PTR)localTail;
					TailDefer_logArr[localTailDeferSeqNum].TID = myTID;
#endif
				}

				continue;
			
			}
			
			if (InterlockedCompareExchangePointer((PVOID*)&localTailAddress->_next, exchangeNode, nullptr) == nullptr)
			{
				CAS1Success = true;
				if (InterlockedCompareExchangePointer((PVOID*)&_tail, exchangeNode, localTail) == localTail)
					CAS2Success = true;

#ifdef __LOGDEBUG__
				localSeqNum = InterlockedIncrement(&g_SeqNum);
				
				logArr[localSeqNum].seqNum = localSeqNum;
				logArr[localSeqNum].TID = myTID;
				logArr[localSeqNum].funcType = ENQUEUE;
				logArr[localSeqNum].firstCasParam2 = (ULONG_PTR)exchangeNode;
				logArr[localSeqNum].firstCasParam3 = (ULONG_PTR)nullptr;
				logArr[localSeqNum].SecondCasParam2 = (ULONG_PTR)exchangeNode;
				logArr[localSeqNum].SecondCasParam3 = (ULONG_PTR)localTail;
				logArr[localSeqNum].NewNodeAddress = (ULONG_PTR)newNode;
				logArr[localSeqNum].CAS1Success = CAS1Success;
				logArr[localSeqNum].CAS2Success = CAS2Success;
				

				if (CAS2Success == false)
				{
					unsigned long long CasSeqNum = InterlockedIncrement(&g_CASERROR_SeqNum);



					CASERROR_logArr[CasSeqNum] = localSeqNum;

				}
#endif
				InterlockedIncrement(&_size);

				break;
			}
		}

	}

	T Dequeue()
	{
		Node* localHead;
		Node* localTail;
		Node* localHeadAddress;
		Node* localTailAddress;
		Node* nextNode;
		Node* nextNodeAddress;
		T retval;
		unsigned long long localSeqNum;
		DWORD myTID;
		bool CAS1Success;

		long long localSize = (long long)InterlockedDecrement(&_size);
		if (localSize < 0)
		{
			InterlockedIncrement(&_size);
			return -1;
		}




		myTID = GetCurrentThreadId();



		while (1)
		{
			CAS1Success = false;
			localHead = _head;
			localTail = _tail;

			localHeadAddress = (Node*)((ULONG_PTR)localHead & MASKING_VALUE17BIT);
			localTailAddress = (Node*)((ULONG_PTR)localTail & MASKING_VALUE17BIT);
			nextNode = localHeadAddress->_next;

			if (nextNode == (Node*)nullptr || nextNode == (Node*)RETURN_NEXTVALUE)
				continue;

			if (localTailAddress->_next != nullptr)
			{

				if(InterlockedCompareExchangePointer((PVOID*)&_tail, localTailAddress->_next, localTail) == localTail)
				{
#ifdef __LOGDEBUG__
					ULONG64 localTailDeferSeqNum = InterlockedIncrement(&g_TailDefer_SeqNum);
					TailDefer_logArr[localTailDeferSeqNum].CAS1Success = true;
					TailDefer_logArr[localTailDeferSeqNum].funcType = DEQUEUE;
					TailDefer_logArr[localTailDeferSeqNum].firstCasParam2 = (ULONG_PTR)localTailAddress->_next;
					TailDefer_logArr[localTailDeferSeqNum].firstCasParam3 = (ULONG_PTR)localTail;
					TailDefer_logArr[localTailDeferSeqNum].TID = myTID;
#endif
				}
				continue;
			}
			


			nextNodeAddress = (Node*)((ULONG_PTR)nextNode & MASKING_VALUE17BIT);
			retval = nextNodeAddress->_data;

			if (InterlockedCompareExchangePointer((PVOID*) & _head, nextNode, localHead) == localHead)
			{
				CAS1Success = true;
				
#ifdef __LOGDEBUG__
				localSeqNum = InterlockedIncrement(&g_SeqNum);
				logArr[localSeqNum].seqNum = localSeqNum;
				logArr[localSeqNum].TID = myTID;
				logArr[localSeqNum].funcType = DEQUEUE;
				logArr[localSeqNum].firstCasParam2 = (ULONG_PTR)nextNode;
				logArr[localSeqNum].firstCasParam3 = (ULONG_PTR)localHead;
				logArr[localSeqNum].DeleteNodeAddress = (ULONG_PTR)localHead;
				logArr[localSeqNum].CAS1Success = CAS1Success;
#endif
				

				if (InterlockedCompareExchangePointer((PVOID*)&localHeadAddress->_next, nullptr, nullptr) == nullptr)
				{
					__debugbreak();
				}

				//delete localHeadAddress;
				_mPool.Delete(localHeadAddress);
				break;
			}
		}

		return retval;
	}

};

#ifdef __LOGDEBUG__
void WriteAllLogData();
#endif