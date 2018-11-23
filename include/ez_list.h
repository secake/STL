#include<stdio.h>

//using this namespace to call ez_stl library function
//使用ez_stl中的库函数需要包含此命名空间
namespace ez_stl
{

	template <class T>
	class LinkListElmt
	{
	private:	
		T* data;
		LinkListElmt* prev;
		LinkListElmt* next;
		//init function, only constructor can use this function -- 初始化函数，只有构造函数才能调用此函数
		void myselfInit(T* t_data, LinkListElmt<T>* t_prev, LinkListElmt<T>* t_next);
	public:
		LinkListElmt();
		LinkListElmt(T* t_data, LinkListElmt<T>* t_prev);
		LinkListElmt(T* t_data, LinkListElmt<T>* t_next);
		LinkListElmt(t* t_data, linklistelmt<t>* t_prev, linklistelmt<t>* t_next);
	};

	template <class T>
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
	
	//private function
	template <class T>
	void LinkListElmt::myselfInit(T* t_data, LinkListElmt<T>* t_prev, LinkListElmt<T>* t_next)
	{
		data = t_data;
		prev = t_prev;
		next = t_next;
	}	
	


	/*************************beautiful dividing line -- 优美分界线************************************/

	/* the following code is fot robustness,do not have core logic*/
	/* 以下为一些为了鲁棒性所写的代码，不涉及核心逻辑 */


	/** start constructor*/

	//init data to null
	template<class T>
	LinkListElmt::LinkListElmt()
	{
		myselfInit(NULL, NULL, NULL);
	}
	
	
	//init data, and init in back of other element -- 此函数用来初始化元素，初始化在其他元素后面
	//T* t_data , ponit to element data -- 指向元素的数据的一个指针
	//LinkListElmt* t_prev, point to prev element -- 指向上一个元素
	template<class T>
	LinkListElmt::LinkListElmt(T* t_data, LinkListElmt<T>* t_prev)
	{
		myselfInit(t_data, t_prev, NULL);
	}

	//init data,and init in front of other element -- 此函数用来初始化元素，初始化在其他函数前面
	//T* t_data , ponit to element data -- 指向元素的数据的一个指针
	//LinkListElmt* t_next, point to next element -- 指向下一个元素
	template<class T>
	LinkListElmt::LinkListElmt(T* t_data, LinkListElmt<T>* t_next)
	{
		myselfInit(t_data, NULL, t_next);
	}
	
	//init data -- 构造函数
	//T* t_data , ponit to element data -- 指向元素的数据的一个指针
	//LinkListElmt* t_prev, point to prev element -- 指向上一个元素
	//LinkListElmt* t_next, point to next element -- 指向下一个元素
	template<class T>
	LinkListElmt::LinkListElmt(T* t_data, LinkListElmt<T>* t_prev, LinkListElmt<T>* t_next)
	{
		myselfInit(t_data, t_prev, t_next);
	}
	/** end constructor*/

} 

