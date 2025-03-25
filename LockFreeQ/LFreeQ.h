#pragma once
#include <windows.h>
#include <iostream>
#include "LMemoryPool.h"

#define MASKING_VALUE17BIT 0x00007fffffffffff
#define LOG_MAXNUM 50'000'000
#define ENQUEUE 0x00
#define DEQUEUE 0x01
#define BIT_64 64
#define UNUSED_BIT 17
#define BIT_OR_VALUE (BIT_64 - UNUSED_BIT)

#define NEWNEXT_VALUE 0x0000000000000001


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

extern LogData* logArr;


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
		_head = (Node*)_mPool.Alloc();
		_head->_next = nullptr;
		_head->_data = 0;
		_tail = _head;
		_size = 0;
		_bitCount = 0;

		logArr = (LogData*)malloc(sizeof(LogData) * LOG_MAXNUM);
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


		localBitCount = InterlockedIncrement(&_bitCount);
		myTID = GetCurrentThreadId();

		newNode = (Node*)_mPool.Alloc();
		newNode->_data = pData;
		newNode->_next = (Node*)NEWNEXT_VALUE;

		exchangeNode = (Node*)((ULONG_PTR)newNode | (localBitCount << BIT_OR_VALUE)); //상위 17비트는 0일 거라는 가정 하에 진행



		while (1)
		{
			localTail = _tail;
			localTailAddress = (Node*)((ULONG_PTR)localTail & MASKING_VALUE17BIT);
			nextNode = localTailAddress->_next;

			if (InterlockedCompareExchangePointer((PVOID*) & localTailAddress->_next, exchangeNode, nullptr) == nextNode)
			{
				if (InterlockedCompareExchangePointer((PVOID*) & _tail, exchangeNode, localTail) != localTail)
					__debugbreak();

				newNode->_next = nullptr;
				localSeqNum = InterlockedIncrement(&g_SeqNum);

				
				///*
				logArr[localSeqNum].seqNum = localSeqNum;
				logArr[localSeqNum].TID = myTID;
				logArr[localSeqNum].funcType = ENQUEUE;
				logArr[localSeqNum].threadNext = (ULONG_PTR)exchangeNode;
				logArr[localSeqNum].nodeNext = (ULONG_PTR)localTailAddress->_next;
				logArr[localSeqNum].myAddress = (ULONG_PTR)localTail;
				//*/


				
				InterlockedIncrement(&_size);

				break;
			}
		}

	}

	T Dequeue()
	{
		Node* localHead;
		Node* localHeadAddress;
		Node* nextNode;
		Node* nextNodeAddress;
		T retval;
		unsigned long long localSeqNum;
		DWORD myTID;
		
		long long localSize = (long long)InterlockedDecrement(&_size);
		if (localSize < 0)
		{
			InterlockedIncrement(&_size);
			return -1;
		}




		myTID = GetCurrentThreadId();



		while (1)
		{
			localHead = _head;
			localHeadAddress =(Node*)((ULONG_PTR)_head & MASKING_VALUE17BIT);
			nextNode = localHeadAddress->_next;

			if (nextNode == (Node*)nullptr || nextNode == (Node*)NEWNEXT_VALUE)
				continue;


			nextNodeAddress = (Node*)((ULONG_PTR)nextNode & MASKING_VALUE17BIT);
			retval = nextNodeAddress->_data;

			if (InterlockedCompareExchangePointer((PVOID*) & _head, nextNode, localHead) == localHead)
			{

				//*
				localSeqNum = InterlockedIncrement(&g_SeqNum);
				logArr[localSeqNum].seqNum = localSeqNum;
				logArr[localSeqNum].TID = myTID;
				logArr[localSeqNum].funcType = DEQUEUE;
				logArr[localSeqNum].threadNext = (ULONG_PTR)nextNode;
				logArr[localSeqNum].nodeNext = (ULONG_PTR)localHeadAddress->_next;
				logArr[localSeqNum].myAddress = (ULONG_PTR)localHead;
			


				//*/


				_mPool.Delete(localHeadAddress);
				break;
			}
		}

		return retval;
	}

};

void WriteAllLogData();
