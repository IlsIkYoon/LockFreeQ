#pragma once
#include <windows.h>
#include "LMemoryPool.h"

#define MASKING_VALUE17BIT 0x00007fffffffffff
#define LOG_MAXNUM 50'000'000
#define ENQUEUE 0x00
#define DEQUEUE 0x01

extern unsigned long long g_SeqNum;

struct LogData
{
	char funcType;
	DWORD TID;

	unsigned long long seqNum;

	ULONG_PTR myAddress;
	ULONG_PTR threadNext;
	ULONG_PTR nodeNext;

	DWORD createCount;
	DWORD deleteCount;
};

extern LogData* g_logArr;


template <typename T>
class LFreeQ
{
	struct Node
	{
		T _data;
		Node* next;

	};

	Node* _head;
	Node* _tail;
	LMemoryPool<T> _mPool;
	unsigned long long _size;
	unsigned long long _bitCount;

public:

	LFreeQ()
	{
		_head = _mPool.Alloc();
		_head->next = nullptr;
		_head->_data = 0;
		_tail = _head;
		_size = 0;
		_bitCount = 0;

		g_logArr = (LogData*)malloc(sizeof(LogData) * LOG_MAXNUM);
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

		myTID = GetCurrentThreadId();

		newNode = _mPool.Alloc();
		newNode->_data = pData;
		newNode->next = nullptr;

		localBitCount = InterlockedIncrement(&_bitCount);
		exchangeNode = (Node*)((ULONG_PTR)newNode | (localBitCount << (64 - 17))); //상위 17비트는 0일 거라는 가정 하에 진행



		while (1)
		{
			localTail = _tail;
			nextNode = localTail->next;


			if (InterlockedCompareExchange(&nextNode, newNode, nullptr) == nullptr)
			{
				InterlockedCompareExchange(&_tail, newNode, localTop);
				localSeqNum = InterlockedIncrement(&g_SeqNum);
				localTailAddress = (Node*)((ULONG_PTR)localTail & MASKING_VALUE17BIT);

				logArr[mySeqNum].seqNum = localSeqNum;
				logArr[mySeqNum].TID = myTID;
				logArr[mySeqNum].funcType = ENQUEUE;
				logArr[mySeqNum].threadNext = (ULONG_PTR)exchangeNode;
				logArr[mySeqNum].nodeNext = (ULONG_PTR)localTailAddress->_next;
				logArr[mySeqNum].myAddress = (ULONG_PTR)localTail;
				logArr[mySeqNum].createCount = ++(localTailAddress->_createCount); //todo// 노드에 없음
				logArr[mySeqNum].deleteCount = localTailAddress->_deleteCount; //todo// 노드에 없음



				InterlockedIncrement(&_size);

				break;
			}
		}

	}

	T Dequeue()
	{
		Node* localHead;
		Node* nextNode;
		T retval;
		unsigned long long localSeqNum;
		DWORD myTID;

		myTID = GetCurrentThreadId();



		while (1)
		{
			localHead = _head;
			nextNode = localHead->next;
			retval = nextNode->_data;

			if (InterlockedCompareExchange(&_head, nextNode, localHead) == localHead)
			{
				localSeqNum = InterlockedIncrement(&g_seqNum);
				logArr[mySeqNum].seqNum = localSeqNum;
				logArr[mySeqNum].TID = myTID;
				logArr[mySeqNum].funcType = DEQUEUE;
				logArr[mySeqNum].threadNext = (ULONG_PTR)nextNode;
				logArr[mySeqNum].nodeNext = (ULONG_PTR)realTopAdd->_next;
				logArr[mySeqNum].myAddress = (ULONG_PTR)localTop;
				logArr[mySeqNum].createCount = realTopAdd->_createCount;
				logArr[mySeqNum].deleteCount = ++(realTopAdd->_deleteCount);




				_mPool.Delete(localHead);
				break;
			}
		}


	}




};
