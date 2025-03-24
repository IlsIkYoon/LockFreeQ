#pragma once
#include <windows.h>

#define LOG_MAXNUM 50'000'000
#define MASKING_VALUE17BIT 0x00007fffffffffff


template <typename T>
class LMemoryPool
{


	struct Node
	{

		T _data;
		Node* _next;
	};

	Node* _top;
	//LONG _size;
	unsigned long long _count;


public:

	LMemoryPool()
	{
		_top = nullptr;
		//_size = 0;
		_count = 0;

	}



	void Delete(void* pNode) //스택의 push동작
	{

		Node* exchangeNode;
		Node* localTop;
		Node* newNode;

		unsigned long long localCount;

		newNode = (Node*)pNode;
		localCount = InterlockedIncrement(&_count);
		exchangeNode = (Node*)((ULONG_PTR)newNode | (localCount << (64 - 17))); //상위 17비트는 0일 거라는 가정 하에 진행


		while (1)
		{
			localTop = _top;
			newNode->_next = localTop;

			if (InterlockedCompareExchangePointer((PVOID*)&_top, exchangeNode, localTop) == localTop)
			{
				//	InterlockedIncrement(&_size);
				break;
			}

		}

	}

	void* Alloc() //스택의 pop 동작
	{
		Node* localTop;
		Node* realTopAdd;
		Node* retNode;

		while (1)
		{


			localTop = _top;
			if (localTop == nullptr)
			{
				//새로 할당해서 주고 return
				Node* newNode = new Node;
				return (void*)newNode;
			}

			realTopAdd = (Node*)((ULONG_PTR)localTop & MASKING_VALUE17BIT);


			Node* nextNode = realTopAdd->_next;

			if (InterlockedCompareExchangePointer((PVOID*)&_top, nextNode, localTop) == localTop)
			{
				retNode = (Node*)((ULONG_PTR)localTop & MASKING_VALUE17BIT);

				//InterlockedDecrement(&_size);

				return (void*)retNode;
				break;
			}

		}

	}

};