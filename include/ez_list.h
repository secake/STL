#include<stdio.h>

//using this namespace to call ez_stl library function
//使用ez_stl中的库函数需要包含此命名空间
namespace ez_stl
{

	template <T>
	struct LinkListElmt
	{
		T* data;
		LinkListElmt* prev;
		LinkListElmt* next;
	}

	template <T>
	class List
	{
	public:
		List();
		~List();
		
		T* getHead();
		T* getTail();
		T* getNext();
		T* getPrev();
		
		int addElmt();
		int removeElmt();
		int initList();
		int destoryList();

		
	}

} 
