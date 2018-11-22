#include<stdio.h>

//using this namespace to call ez_stl library function
//使用ez_stl中的库函数需要包含此命名空间
namespace ez_stl
{

	template <T>
	class LinkListElmt
	{
	private:	
		T* data;
		LinkListElmt* prev;
		LinkListElmt* next;

		
	public:
		LinkListElmt();
		LinkListElmt(T* t_data, LinkListElmt<T>* t_next);
		LinkListElmt(T* t_data, LinkListElmt<T>* t_prev, LinkListElmt<T>* t_next);
	};

	template <T>
	class List
	{
	private:
		int size;//element number-- 列表元素个数
		LinkListElmt<T>* head;//linklist head point -- 链式表头元素指针
		LinkListElmt<T>* tail;//linklist tail point -- 链式表尾元素指针

	public:
		List();
		~List();
		
		T* getHead();
		T* getTail();
		T* getNext();
		T* getPrev();
		
		int initList();
		int addElmt();
		int removeElmt();
		int destoryList();

		
	};

	/*************************beautiful dividing line -- 优美分界线************************************/

	/* the following code is fot robustness,do not have core logic*/
	/* 以下为一些为了鲁棒性所写的代码，不涉及核心逻辑 */


	/** init LinkListElmt data*/
	/** 初始化链表元素里面的数据*/

	//init data to null
	template<T>
	LinkListElmt()
	{
		data = NULL;
		prev = NULL;
		next = NULL;
	}
	
	//init data to null,and init in front other element--此函数用来初始化元素，初始化在其他函数前面
	//T* t_data , ponit to element data--指向元素的数据的一个指针
	//LinkListElmt* T_next, point to next element--指向下一个元素
	template<T>
	LinkListElmt(T* t_data, LinkListElmt<T>* t_next)
	{
		data = t_data;
		prev = NULL;
		next = t_next;
	}
	
	template<T>
	LinkListElmt(T* t_data, LinkListElmt<T>* t_prev, LinkListElmt<T>* t_next)
	{
			data = t_data;
			prev = t_prev;
			prev = t_next;
	}

} 

