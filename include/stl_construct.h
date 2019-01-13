//此头文件负责对象内容的构造与析构
//与隔壁的stl_alloc.h不同，stl_alloc.h用于负责内存空间的配置和释放


GI_STL_INTERNAL_CONSTRUCT_H
#define __SGI_STL_INTERNAL_CONSTRUCT_H

#include <new.h>

__STL_BEGIN_NAMESPACE

/* 以下的函数名只会被SGI STL调用
 * 用户级不要调用此API
 */


//对已经由stl_alloc.h中分配好的内存进行初始化工作
//	_T1*		__p		,要操作的内存指针，已经申请好内存
//	const _t2&	__value	,要初始化的值;为了减少传参过程的多余拷贝，所以这里是传引用，又因为不希望函数内会对__value的值进行改变，所以前面有const修饰符
template <class _T1, class _T2>
inline void _Construct(_T1* __p, const _T2& __value) {
	//注意：这个new是placement new，即对已经存在的内存进行初始化，并不进行内存分配，下同
	//
	//placement new 做了三件事：
	//_T1* tmp;
	//void* mem = operator new();	//这里借用分配好的内存
	//tmp = static_cast<_T1*>(mem);	//对内存进行安全转换
	//tmp->_T1::_T1(__value);		//调用构造函数
	//
	//真实作用是调用_T1的构造函数，即T1:T1(value);
	new ((void*) __p) _T1(__value);
}

//重载上面的函数，这里省去了初值
template <class _T1>
inline void _Construct(_T1* __p) {
	  new ((void*) __p) _T1();
}

//第一个版本的_Destory
//接受一个指针的析构函数，直接调用对应类型的析构函数
template <class _Tp>
inline void _Destroy(_Tp* __pointer) {
	  __pointer->~_Tp();
}

//下面出现的“可用有效的析构函数”的定义为：
//如果要析构的类的类成员变量中有析够函数，则编译器则默认为此类创建一个析构函数，这个析构函数是可用有效的，用户自定义的析够函数也是可用有效的
//反之，如果要析构的类的类成员变量中没有析构函数，并且类中也没有用户自定义析构函数，则此类没有可用有效的析构函数

//被第二个版本的_Destory调用
//前两个参数，来表示要析构的范围;	
//第三个参数只表示要析构的类型是否有可用有效的析构函数，如果有可用有效的析够函数，则调用此函数进行析构
template <class _ForwardIterator>
void
__destroy_aux(_ForwardIterator __first, _ForwardIterator __last, __false_type)
{
	  for ( ; __first != __last; ++__first)
		    destroy(&*__first);
}

//同样被第二个版本的_Destory调用
//但这个函数不做任何事情，因为要析构的类型没有可用有效的析构函数
template <class _ForwardIterator> 
inline void __destroy_aux(_ForwardIterator, _ForwardIterator, __true_type) {}

//第二个版本作用于此函数
//作用为：判断要析构的类型是否有可用有效的析构函数
//为什么这么做：因为要析构的是一段范围，那么这个范围就有可能很大很大，如果这些变量没有析构的必要，那么就不应该浪费资源去一次次的调用这些析够函数
template <class _ForwardIterator, class _Tp>
inline void 
__destroy(_ForwardIterator __first, _ForwardIterator __last, _Tp*)
{
	//这个__type_traits<_Tp>::has_trivial_destructor函数用来判断要析构的类型是否有可用有效析构函数，
	//名称has_trivial_destructor意为是否有无价值的析够函数，
	//所以如果有可用有效的析够函数，则值其实为__false_type
	  typedef typename __type_traits<_Tp>::has_trivial_destructor
		            _Trivial_destructor;
	  //调用两种可能相对应的函数，在上面实现;
	  //如果有可用有效的析够函数，则调用__false_type对应的函数
	  //如果没有可用有效的析构函数，则调用__true_type对应的函数
	    __destroy_aux(__first, __last, _Trivial_destructor());
}

//第二个版本的_Destory
//接受两个迭代器，来表示要析构的范围
//其实这个函数相当于一个适配器，为了包装接口
template <class _ForwardIterator>
inline void _Destroy(_ForwardIterator __first, _ForwardIterator __last) {
	//调用__destory函数，并包装第三个参数为要析构变量的类型类型
	  __destroy(__first, __last, __VALUE_TYPE(__first));
}

//如下一些函数其实为一些特殊类型的特化处理
inline void _Destroy(char*, char*) {}
inline void _Destroy(int*, int*) {}
inline void _Destroy(long*, long*) {}
inline void _Destroy(float*, float*) {}
inline void _Destroy(double*, double*) {}
#ifdef __STL_HAS_WCHAR_T
inline void _Destroy(wchar_t*, wchar_t*) {}
#endif /* __STL_HAS_WCHAR_T */


