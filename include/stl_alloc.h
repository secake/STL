//此头文件负责对内存空间的配置和释放
//与隔壁的stl_construct.h不同，stl_construct.h用于负责对象内容的构造与析构

/* 以下的函数名只会被SGI STL调用
 * 用户级不要调用此API
 */

#ifndef __SGI_STL_INTERNAL_ALLOC_H
#define __SGI_STL_INTERNAL_ALLOC_H

#ifdef __SUNPRO_CC
#  define __PRIVATE public
   // Extra access restrictions prevent us from really making some things
   // private.
#else
#  define __PRIVATE private
#endif


//define __USE_MALLOC之后将不会调用二级分配器，只会调用一级分配器
#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
#  define __USE_MALLOC
#endif

//定义__THROW_BAD_ALLOC的行为，即抛出alloc异常的行为
#ifndef __THROW_BAD_ALLOC
#  if defined(__STL_NO_BAD_ALLOC) || !defined(__STL_USE_EXCEPTIONS)
#    include <stdio.h>
#    include <stdlib.h>
#    define __THROW_BAD_ALLOC fprintf(stderr, "out of memory\n"); exit(1)
#  else /* Standard conforming out-of-memory handling */
#    include <new>
#    define __THROW_BAD_ALLOC throw std::bad_alloc()
#  endif
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifndef __RESTRICT
#  define __RESTRICT
#endif

#ifdef __STL_THREADS
# include <stl_threads.h>
# define __NODE_ALLOCATOR_THREADS true
# ifdef __STL_SGI_THREADS
  // We test whether threads are in use before locking.
  // Perhaps this should be moved into stl_threads.h, but that
  // probably makes it harder to avoid the procedure call when
  // it isn't needed.
    extern "C" {
      extern int __us_rsthread_malloc;
    }
	// The above is copied from malloc.h.  Including <malloc.h>
	// would be cleaner but fails with certain levels of standard
	// conformance.
#   define __NODE_ALLOCATOR_LOCK if (threads && __us_rsthread_malloc) \
                { _S_node_allocator_lock._M_acquire_lock(); }
#   define __NODE_ALLOCATOR_UNLOCK if (threads && __us_rsthread_malloc) \
                { _S_node_allocator_lock._M_release_lock(); }
# else /* !__STL_SGI_THREADS */
#   define __NODE_ALLOCATOR_LOCK \
        { if (threads) _S_node_allocator_lock._M_acquire_lock(); }
#   define __NODE_ALLOCATOR_UNLOCK \
        { if (threads) _S_node_allocator_lock._M_release_lock(); }
# endif
#else
//  Thread-unsafe
#   define __NODE_ALLOCATOR_LOCK
#   define __NODE_ALLOCATOR_UNLOCK
#   define __NODE_ALLOCATOR_THREADS false
#endif

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#endif


//这里初始化 分配器在无法分配内存时，即malloc失败时的处理函数为 0,
//用户可以通过set_malloc_handler() 函数来设置自己的内存不足处理函数
#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
# ifdef __DECLARE_GLOBALS_HERE
    void (* __malloc_alloc_oom_handler)() = 0;
    // g++ 2.7.2 does not handle static template data members.
# else
    extern void (* __malloc_alloc_oom_handler)();
# endif
#endif

//__malloc_alloc_template 一级分配器，通常比二级分配器慢，但是对空间的运用比较高级
//直接使用malloc来分配内存，与之对于，SGI还有二级分配器，当有二级分配器时，一级分配器只处理128字节以上的内存分配
template <int __inst>
class __malloc_alloc_template {

private:

	//这里定义内存不足时的malloc回调和realloc的回调函数
  static void* _S_oom_malloc(size_t);
  static void* _S_oom_realloc(void*, size_t);

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
  //这个是内存不足时的处理函数，由用户设置，因为内存不足时要处理的事情是用户的责任，这里借鉴了C++ new handler机制，
  //oom即为out of memory
  //但是没有使用::operator new 的原因是c++并没有提供类似realloc功能的函数接口。::operator new的功能和malloc的功能完全相同
  static void (* __malloc_alloc_oom_handler)();
#endif

public:

  //提供stl的标准接口allocate() deallocate()，直接使用malloc()，realloc()和free()，并考虑内存不足的情况
  static void* allocate(size_t __n)
  {
    void* __result = malloc(__n);
    if (0 == __result) __result = _S_oom_malloc(__n);
    return __result;
  }

  static void deallocate(void* __p, size_t /* __n */)
  {
    free(__p);
  }

  static void* reallocate(void* __p, size_t /* old_sz */, size_t __new_sz)
  {
    void* __result = realloc(__p, __new_sz);
    if (0 == __result) __result = _S_oom_realloc(__p, __new_sz);
    return __result;
  }

  //调用此函数来设置__malloc_alloc_oom_handler()，内存不足处理函数
  static void (* __set_malloc_handler(void (*__f)()))()
  {
    void (* __old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = __f;
    return(__old);
  }

};

// 初始化没有内存不足处理函数

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int __inst>
void (* __malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = 0;
#endif

//allocate()失败时调用此函数，此函数调用用户自定义内存不足处理函数，并重新分配内存，一直循环，知道分配到内存即可。
//如果没有用户自定义处理函数，则直接抛出异常
template <int __inst>
void*
__malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for (;;) {
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*__my_malloc_handler)();
        __result = malloc(__n);
        if (__result) return(__result);
    }
}

//reallocate()失败时调用，原理和上面_S_oom_malloc相同
template <int __inst>
void* __malloc_alloc_template<__inst>::_S_oom_realloc(void* __p, size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for (;;) {
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*__my_malloc_handler)();
        __result = realloc(__p, __n);
        if (__result) return(__result);
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;

//这是容器调用分配器的真正接口，为了符合stl标准的接口，只是调用allocate() reallocate() deallocate()
template<class _Tp, class _Alloc>
class simple_alloc {

public:
    static _Tp* allocate(size_t __n)
      { return 0 == __n ? 0 : (_Tp*) _Alloc::allocate(__n * sizeof (_Tp)); }
    static _Tp* allocate(void)
      { return (_Tp*) _Alloc::allocate(sizeof (_Tp)); }
    static void deallocate(_Tp* __p, size_t __n)
      { if (0 != __n) _Alloc::deallocate(__p, __n * sizeof (_Tp)); }
    static void deallocate(_Tp* __p)
      { _Alloc::deallocate(__p, sizeof (_Tp)); }
};

// 配置器的配接器，每配置一块内存，都会比需求多出8字节来
template <class _Alloc>
class debug_alloc {

private:

  enum {_S_extra = 8};  // Size of space used to store size.  Note
                        // that this must be large enough to preserve
                        // alignment.

public:

  static void* allocate(size_t __n)
  {
    char* __result = (char*)_Alloc::allocate(__n + (int) _S_extra);
    *(size_t*)__result = __n;
    return __result + (int) _S_extra;
  }

  static void deallocate(void* __p, size_t __n)
  {
    char* __real_p = (char*)__p - (int) _S_extra;
    assert(*(size_t*)__real_p == __n);
    _Alloc::deallocate(__real_p, __n + (int) _S_extra);
  }

  static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz)
  {
    char* __real_p = (char*)__p - (int) _S_extra;
    assert(*(size_t*)__real_p == __old_sz);
    char* __result = (char*)
      _Alloc::reallocate(__real_p, __old_sz + (int) _S_extra,
                                   __new_sz + (int) _S_extra);
    *(size_t*)__result = __new_sz;
    return __result + (int) _S_extra;
  }

};


# ifdef __USE_MALLOC

typedef malloc_alloc alloc;
typedef malloc_alloc single_client_alloc;

# else


//以下是二级分配器
//二级分配器来分配小于128字节的内存,其他情况由一级分配器来分配
//目的是：为了减少内存碎片的问题，和零散内存的分配产生的效率问题

#if defined(__SUNPRO_CC) || defined(__GNUC__)
	//	使用如下枚举来保证分配的内存一定是8的整数倍
	//_MAX_BYTES	是二级分配器所能分配的最大字节
	//_NFREELISTS	是二级分配器中自由链表的个数，8,16,32,40,48,...，128,一共16个自由链表
  enum {_ALIGN = 8};
  enum {_MAX_BYTES = 128};
  enum {_NFREELISTS = 16}; // _MAX_BYTES/_ALIGN
#endif

template <bool threads, int inst>
class __default_alloc_template {

//这里学习到一个很好的习惯是，成员变量都设置为私有，而把部分函数接口公有化

private:
	//这里使用static const int _ALIGN = 8;来代替enum是更好的选择，但是并不是所有编译器都支持前者
#if ! (defined(__SUNPRO_CC) || defined(__GNUC__))
    enum {_ALIGN = 8};
    enum {_MAX_BYTES = 128};
    enum {_NFREELISTS = 16}; // _MAX_BYTES/_ALIGN
# endif
	//使用此函数，使所需字节成为8的整数倍
  static size_t
  _S_round_up(size_t __bytes) 
    { return (((__bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1)); }

	//因为正常情况下维护自由链表需要一个多余的指针，这样就多出了多余没必要的空间分配，很好的解决方法是
	//使用联合，当自由链表维护的时候他是一个指针，当空间被分配之后，它可以和普通空间一样
__PRIVATE:
  union _Obj {
        union _Obj* _M_free_list_link;
        char _M_client_data[1];    /* The client sees this.        */
  };

private:
  //来定义16个自由链表,每个链表分别用来分配8,16,24,32,40...,128的空间
# if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
    static _Obj* __STL_VOLATILE _S_free_list[]; 
        // Specifying a size results in duplicate def for 4.1
# else
    static _Obj* __STL_VOLATILE _S_free_list[_NFREELISTS]; 
# endif
	//这个函数用来获取要分配的内存在16个自由链表中的索引，例（63+8-1）/8-1=7,正好是64在_S_free_list中的索引
  static  size_t _S_freelist_index(size_t __bytes) {
        return (((__bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1);
  }

  //当请求的自由链表无法分配空间时，使用此函数来为自由链表填充空间，新的空间将来自于内存池
  //规则是内存池填充19个__n大小的内存(这里说明下，stl中的内存全都是从heap即堆中分配的)，并返回1个__n大小的内存给申请者
  //如果内存池中的内存小于20个__n大小，但是大于1个__n大小时，则将剩余的大于1个__n大小的内存全部返回给自由链表
  //如果内存池中的内存连一个__n大小都无法分配时，则先会将内存池中剩余的全部内存分配给相同大小的自由链表（因为剩余的内存一定小于128字节，并且一定是8的倍数，所以不会出现内存泄漏的情况）
  //清空内存池之后则调用malloc分配40个__n大小的内存，1个返回给申请者，19个返回给自由链表，剩余20个存储在内存池中
  //如果连malloc都调用失败的话，那么会先在自由链表中查找大于__n的自由链表，如果某个大于__n的自由链表中发现了可用内存，则将内存全部归还到内存池，并重新调用refill函数
  //如果连自由链表中都无法提供内存的话，则会调用一级分配器的allocate(),把内存不足情况的处理来交给一级分配器
  static void* _S_refill(size_t __n);
  //这个函数才真正的发挥从内存池中取空间到自由链表
  static char* _S_chunk_alloc(size_t __size, int& __nobjs);

  //内存池起始指针
  static char* _S_start_free;
  //内存池终止指针
  static char* _S_end_free;
  //内存池容量/水深
  static size_t _S_heap_size;

# ifdef __STL_THREADS
    static _STL_mutex_lock _S_node_allocator_lock;
# endif 

	//这里定义锁类，即线程锁
    class _Lock;
    friend class _Lock;
    class _Lock {
        public:
            _Lock() { __NODE_ALLOCATOR_LOCK; }
            ~_Lock() { __NODE_ALLOCATOR_UNLOCK; }
    };

public:

  //__n必须大于0,这里在simple_alloc中已经过滤过了
  static void* allocate(size_t __n)
  {
    void* __ret = 0;
	//如果大于128字节，则调用一级配置器
    if (__n > (size_t) _MAX_BYTES) {
      __ret = malloc_alloc::allocate(__n);
    }
    else {
		//定位到从哪个自由链表分配内存
      _Obj* __STL_VOLATILE* __my_free_list
          = _S_free_list + _S_freelist_index(__n);

#     ifndef _NOTHREADS
      /*REFERENCED*/
      _Lock __lock_instance;
#     endif
	  //__RESTRICT修饰符确保编译器不会对此变量进行编译器优化，即每次调用时都老老实实从内存中取，在多线程的环境下，某些编译器进行的优化会产生意想不到的后果
      _Obj* __RESTRICT __result = *__my_free_list;
	  //如果自由链表空，则调用refill()函数填充自由链表，并由refill()函数返回一块内存给申请者
	  //否则从自由链表清出一个结点并返回给申请者
      if (__result == 0)
        __ret = _S_refill(_S_round_up(__n));
      else {
		  //这里完全可以写成*__my_free_list = *__my_free_list -> _M_free_list_link;而且使用了__result后增加了空间的分配，
		  //但是这样做的好处是使代码表达更清楚，观赏性更佳，变量什么作用就要用什么名字
        *__my_free_list = __result -> _M_free_list_link;
        __ret = __result;
      }
    }
	//返回空间给申请者
    return __ret;
  };

  //这里回收自由链表配置出去的空间，因为配置器的作用既要配置内存，也要回收内存
  //这里的__n可能会为0
  //回收后的内存依旧归还到自由链表
  static void deallocate(void* __p, size_t __n)
  {
		//如果大小超过128字节，则由一级配置器回收，谁配置谁回收原则
    if (__n > (size_t) _MAX_BYTES)
      malloc_alloc::deallocate(__p, __n);
    else {
		//找到要回收的内存应该回收到哪个自由链表
      _Obj* __STL_VOLATILE*  __my_free_list
          = _S_free_list + _S_freelist_index(__n);
      _Obj* __q = (_Obj*)__p;

      //这里加上线程锁
#       ifndef _NOTHREADS
      /*REFERENCED*/
      _Lock __lock_instance;
#       endif /* _NOTHREADS */
	  //这里是自由链表回收空间的代码
      __q -> _M_free_list_link = *__my_free_list;
      *__my_free_list = __q;

		//线程锁实例为局部遍历，在此析构，所以也在此处解锁
		//多么精简的代码！
	}
  }

  static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz);

} ;

//线程安全
//所谓线程安全就是是否可以让多个用户线程同时访问此函数
typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;
//线程不安全
typedef __default_alloc_template<false, 0> single_client_alloc;

//重载二级配置器的==，!=操作符，用来判断两个配置器是否相同，因为参数都传进来了，所以类型肯定相同
//但这么做的原因是下面还有很多这样的重载
template <bool __threads, int __inst>
inline bool operator==(const __default_alloc_template<__threads, __inst>&,
                       const __default_alloc_template<__threads, __inst>&)
{
  return true;
}

# ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <bool __threads, int __inst>
inline bool operator!=(const __default_alloc_template<__threads, __inst>&,
                       const __default_alloc_template<__threads, __inst>&)
{
  return false;
}
# endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */



/*从内存池分配空间到自由链表的函数
 * size_t __size,	即为8,16,24...等等
 * int& __nobjs,	表示分配多少个__size大小的内存，refill()中设置了此值为20。
 *					！注意这里使用的是引用&，因为这里可能会修改他的值，并想让调用者知道他的修改
 *
 * 使用内存池的好处是：
 *	减少内存malloc和free的次数，减少消耗
 *	解决小型区块内存分配产生的内存碎片问题，因为每次都进行大型内存分配，并且保证均为8的倍数
 * */
template <bool __threads, int __inst>
char*
__default_alloc_template<__threads, __inst>::_S_chunk_alloc(size_t __size, 
                                                            int& __nobjs)
{
    char* __result;
	//总需要的空间大小
    size_t __total_bytes = __size * __nobjs;
	//内存池剩余的空间大小
    size_t __bytes_left = _S_end_free - _S_start_free;

	//如果内存池充足，则直接返回内存，并由refill设置20个结点的逻辑关系并填充到自由链表
    if (__bytes_left >= __total_bytes) {
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    }//否则如果大于一个__size大小，则将内存返回，并修改__nobjs的大小 
	else if (__bytes_left >= __size) {
        __nobjs = (int)(__bytes_left/__size);
        __total_bytes = __size * __nobjs;
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
	}//如果连一个__size的大小都无法分配了
	else {
		//则准备分配2个__total_bytes个大小，即40个__size大小
        size_t __bytes_to_get = 
	  2 * __total_bytes + _S_round_up(_S_heap_size >> 4);

		//如果内存池不空，则将内存池中的剩余全部空间分配给适应的自由链表
        if (__bytes_left > 0) {
            _Obj* __STL_VOLATILE* __my_free_list =
                        _S_free_list + _S_freelist_index(__bytes_left);

            ((_Obj*)_S_start_free) -> _M_free_list_link = *__my_free_list;
            *__my_free_list = (_Obj*)_S_start_free;
        }
		//置空了内存池之后，进行malloc分配空间到内存池
        _S_start_free = (char*)malloc(__bytes_to_get);
	//如果malloc失败，则代表已经无法从内存池中取出内存，则去大于__size的自由链表中查找是否有剩余内存
        if (0 == _S_start_free) {
            size_t __i;
            _Obj* __STL_VOLATILE* __my_free_list;
	
		//去大于__size的自由链表中查找是否有剩余内存
            for (__i = __size;
                 __i <= (size_t) _MAX_BYTES;
                 __i += (size_t) _ALIGN) {
                __my_free_list = _S_free_list + _S_freelist_index(__i);
                __p = *__my_free_list;
		//如果找到了剩余的内存，则将剩余的内存还回给内存池，并且重新调用此方法，满足此次自由链表的填充需求
                if (0 != __p) {
                    *__my_free_list = __p -> _M_free_list_link;
                    _S_start_free = (char*)__p;
                    _S_end_free = _S_start_free + __i;
                    return(_S_chunk_alloc(__size, __nobjs));
                }
            }
		//很明显，连自由链表中都没有够回收内存了，真是所有内存都用尽了呢！这时候只能够求助于一级配置器了，并把内存不足的处理锅留给一级配置器来背,而且如果用户没有设置一级配置器的oom_handler的话，那么一级配置器不出所料的将抛出异常
	    _S_end_free = 0;
            _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
        }
	//如果一级配置器没有抛出异常，并且配置内存成功了，那可真是个好消息，内存池又可以重新分配了，就继续调用此方法，完成此次自由链表的填充
        _S_heap_size += __bytes_to_get;
        _S_end_free = _S_start_free + __bytes_to_get;
        return(_S_chunk_alloc(__size, __nobjs));
    }
}

//如果自由链表空了的话，会调用这个函数来从内存池取出内存，并将内存填充给自由链表
template <bool __threads, int __inst>
void*
__default_alloc_template<__threads, __inst>::_S_refill(size_t __n)
{
	//当然不能一个一个分配了，那样就完全没有解决内存碎片的问题，二级分配器的好处都没了
	//refill会向内存池申请20个__n大小的空间，注意！申请到的内存可能小于20个__n大小，但一定会大于1个__n大小，
	//并且将一个返回给自由链表的调用函数，由自由链表返还给申请者，剩余的19个会直接填充给自由链表，这也是refill命名的原因
    int __nobjs = 20;
	//向内存池申请内存
    char* __chunk = _S_chunk_alloc(__n, __nobjs);
    _Obj* __STL_VOLATILE* __my_free_list;
    _Obj* __result;
    _Obj* __current_obj;
    _Obj* __next_obj;
    int __i;
	//如果返回的刚刚好只有一个__n大小，则直接返回给自由链表就好了
    if (1 == __nobjs) return(__chunk);

	//定位自由链表
    __my_free_list = _S_free_list + _S_freelist_index(__n);

	//修改19个或更少的结点间的逻辑指向关系
      __result = (_Obj*)__chunk;
	//看这几句感觉其实STL源码的可读性真的挺高的
      *__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
      for (__i = 1; ; __i++) {
        __current_obj = __next_obj;
        __next_obj = (_Obj*)((char*)__next_obj + __n);
	//如果是最后一个结点，则把指针(next指针)指向0，
        if (__nobjs - 1 == __i) {
            __current_obj -> _M_free_list_link = 0;
            break;
        } else {
            __current_obj -> _M_free_list_link = __next_obj;
        }
      }
	//注意这里只返回了一个哦
    return(__result);
}

//reallocate函数，内存不够时从新分配内存吧，例如vector是可变长数组，当元素存储不下时，vector会调用此函数分配内存，vector新分配的内存会是原内存空间的二倍大小
template <bool threads, int inst>
void*
__default_alloc_template<threads, inst>::reallocate(void* __p,
                                                    size_t __old_sz,
                                                    size_t __new_sz)
{
    void* __result;
    size_t __copy_sz;

	//当新空间和旧空间的大小都超过128字节时，视为大空间，则直接调用系统的realloc()函数
    if (__old_sz > (size_t) _MAX_BYTES && __new_sz > (size_t) _MAX_BYTES) {
        return(realloc(__p, __new_sz));
    }
	//如果就空间和新空间在扩大到8的倍数之后大小相同，则代表不需要重新分配空间，直接返回
    if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p);
	//先调用二级分配器的allocate()函数来分配小空间内存，因为没有必要再写一套自由链表，内存池balabala的逻辑
    __result = allocate(__new_sz);
	//判断是旧空间大还是新空间大，把要内存拷贝的大小设置为小的那个。
	//所以当要新申请的空间小于旧的时，则会出现数据丢失，不过没准有人希望这么做呢~
    __copy_sz = __new_sz > __old_sz? __old_sz : __new_sz;
	//进行内存拷贝，没办法，不可能做到不重新申请空间直接扩充容量的行为，只能申请一块大的空间，并把原空间的数据拷贝过来
    memcpy(__result, __p, __copy_sz);
	//都拷贝完了，就空间肯定是要析构掉的了，调用的是二级分配的内存回收函数
    deallocate(__p, __old_sz);
    return(__result);
}

//以下是进行静态变量的初始化工作

//线程加锁初始化balabala等，也不太了解
#ifdef __STL_THREADS
    template <bool __threads, int __inst>
    _STL_mutex_lock
    __default_alloc_template<__threads, __inst>::_S_node_allocator_lock
        __STL_MUTEX_INITIALIZER;
#endif

//初始化内存池空
template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __default_alloc_template<__threads, __inst>::_S_heap_size = 0;

//初始化16个空的自由链表
template <bool __threads, int __inst>
typename __default_alloc_template<__threads, __inst>::_Obj* __STL_VOLATILE
__default_alloc_template<__threads, __inst> ::_S_free_list[
# if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
	//全局函数的枚举，代表自由链表个数，即16
    _NFREELISTS
# else
	//二级配置器中局部变量的枚举，代表自由链表个数，即16
    __default_alloc_template<__threads, __inst>::_NFREELISTS
# endif
	//全部初始化为0
] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
#endif /*这是没有定义user_malloc的情况下的，即使用二级分配器的*/


//下面这个分配器类实现了C++标准
//但SGI只是简单实现了，并没有用，SGI使用的是simple_alloc类
//因为SGI决定把 内存的分配和变量的初始化分开来进行
#ifdef __STL_USE_STD_ALLOCATORS

template <class _Tp>
class allocator {
  typedef alloc _Alloc;          //基础分配器，在SGI STL里即使二级配置器
public:
	//定义了一些别名，并不用特别介绍吧
  typedef size_t     size_type;
  typedef ptrdiff_t  difference_type;
  typedef _Tp*       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;
  typedef _Tp        value_type;

  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };

	//构造析构函数，__STL_NOTHROW定义在stl_config.h中，要么为空，要么会抛出throw异常
  allocator() __STL_NOTHROW {}
  allocator(const allocator&) __STL_NOTHROW {}
  template <class _Tp1> allocator(const allocator<_Tp1>&) __STL_NOTHROW {}
  ~allocator() __STL_NOTHROW {}

	//获取地址，返回指针或常量指针
  pointer address(reference __x) const { return &__x; }
  const_pointer address(const_reference __x) const { return &__x; }

 	// 分配器接口，直接调用二级配置器的接口，__n 是可以为0的，因为c++标准这么说的
  _Tp* allocate(size_type __n, const void* = 0) {
    return __n != 0 ? static_cast<_Tp*>(_Alloc::allocate(__n * sizeof(_Tp))) 
                    : 0;
  }

 	// 分配器内存回收接口，__p 不可以为0
  void deallocate(pointer __p, size_type __n)
    { _Alloc::deallocate(__p, __n * sizeof(_Tp)); }
	//size_t可以保证其大小足以存储内存中所有对象，所以STL中设置可分配的最大空间为size_t字节，即为size_t(-1)/sizeof(_Tp)个_Tp类型变量
  size_type max_size() const __STL_NOTHROW 
    { return size_t(-1) / sizeof(_Tp); }
	//执行简单的consruct和destiry
  void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }
};


//并没有传进任何类型的分配器
template<>
class allocator<void> {
public:
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
  typedef void        value_type;

  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };
};

//重载==运算符，返回两个配置器相等
template <class _T1, class _T2>
inline bool operator==(const allocator<_T1>&, const allocator<_T2>&) 
{
  return true;
}

//重载!=运算符，返回两个配置器相等，因为!=返回值为false就代表==
template <class _T1, class _T2>
inline bool operator!=(const allocator<_T1>&, const allocator<_T2>&)
{
  return false;
}


//对配置器进行适配器，把SGI STL的配置器转换为标准接口
//其中内容基本已经介绍过，不在介绍
template <class _Tp, class _Alloc>
struct __allocator {
  _Alloc __underlying_alloc;	

  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef _Tp*       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;
  typedef _Tp        value_type;

  template <class _Tp1> struct rebind {
    typedef __allocator<_Tp1, _Alloc> other;
  };

  __allocator() __STL_NOTHROW {}
  __allocator(const __allocator& __a) __STL_NOTHROW
    : __underlying_alloc(__a.__underlying_alloc) {}
  template <class _Tp1> 
  __allocator(const __allocator<_Tp1, _Alloc>& __a) __STL_NOTHROW
    : __underlying_alloc(__a.__underlying_alloc) {}
  ~__allocator() __STL_NOTHROW {}

  pointer address(reference __x) const { return &__x; }
  const_pointer address(const_reference __x) const { return &__x; }

  
  _Tp* allocate(size_type __n, const void* = 0) {
    return __n != 0 
        ? static_cast<_Tp*>(__underlying_alloc.allocate(__n * sizeof(_Tp))) 
        : 0;
  }

  
  void deallocate(pointer __p, size_type __n)
    { __underlying_alloc.deallocate(__p, __n * sizeof(_Tp)); }

  size_type max_size() const __STL_NOTHROW 
    { return size_t(-1) / sizeof(_Tp); }

  void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }
};

template <class _Alloc>
class __allocator<void, _Alloc> {
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
  typedef void        value_type;

  template <class _Tp1> struct rebind {
    typedef __allocator<_Tp1, _Alloc> other;
  };
};



//重载==运算符，用于判断两个分配器是否相同
template <class _Tp, class _Alloc>
inline bool operator==(const __allocator<_Tp, _Alloc>& __a1,
                       const __allocator<_Tp, _Alloc>& __a2)
{
  return __a1.__underlying_alloc == __a2.__underlying_alloc;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
//重载!=运算符，用于判断两个分配器是否不同
template <class _Tp, class _Alloc>
inline bool operator!=(const __allocator<_Tp, _Alloc>& __a1,
                       const __allocator<_Tp, _Alloc>& __a2)
{
  return __a1.__underlying_alloc != __a2.__underlying_alloc;
}
#endif 

//重载一级配置器的==和!=运算符
template <int inst>
inline bool operator==(const __malloc_alloc_template<inst>&,
                       const __malloc_alloc_template<inst>&)
{
  return true;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <int __inst>
inline bool operator!=(const __malloc_alloc_template<__inst>&,
                       const __malloc_alloc_template<__inst>&)
{
  return false;
}
#endif 

//debug_alloc的==和!=运算符的重载
template <class _Alloc>
inline bool operator==(const debug_alloc<_Alloc>&,
                       const debug_alloc<_Alloc>&) {
  return true;
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <class _Alloc>
inline bool operator!=(const debug_alloc<_Alloc>&,
                       const debug_alloc<_Alloc>&) {
  return false;
}
#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */



// 配置器的一个配接器，他有两个目的:
// 1:使得容器既可以使用标准风格也可以使用SGI风格的分配器
// 2:提供一种查询一个分配器是否有不同实例的机制。如果没有不同实例，容器可以避免浪费内存来存储空对象

template <class _Tp, class _Allocator>
struct _Alloc_traits
{
  static const bool _S_instanceless = false;
  typedef typename _Allocator::__STL_TEMPLATE rebind<_Tp>::other 
          allocator_type;
};

template <class _Tp, class _Allocator>
const bool _Alloc_traits<_Tp, _Allocator>::_S_instanceless;

// The version for the default allocator.

template <class _Tp, class _Tp1>
struct _Alloc_traits<_Tp, allocator<_Tp1> >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, alloc> _Alloc_type;
  typedef allocator<_Tp> allocator_type;
};

// Versions for the predefined SGI-style allocators.

template <class _Tp, int __inst>
struct _Alloc_traits<_Tp, __malloc_alloc_template<__inst> >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __malloc_alloc_template<__inst> > _Alloc_type;
  typedef __allocator<_Tp, __malloc_alloc_template<__inst> > allocator_type;
};

template <class _Tp, bool __threads, int __inst>
struct _Alloc_traits<_Tp, __default_alloc_template<__threads, __inst> >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __default_alloc_template<__threads, __inst> > 
          _Alloc_type;
  typedef __allocator<_Tp, __default_alloc_template<__threads, __inst> > 
          allocator_type;
};

template <class _Tp, class _Alloc>
struct _Alloc_traits<_Tp, debug_alloc<_Alloc> >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, debug_alloc<_Alloc> > _Alloc_type;
  typedef __allocator<_Tp, debug_alloc<_Alloc> > allocator_type;
};

// Versions for the __allocator adaptor used with the predefined
// SGI-style allocators.

template <class _Tp, class _Tp1, int __inst>
struct _Alloc_traits<_Tp, 
                     __allocator<_Tp1, __malloc_alloc_template<__inst> > >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __malloc_alloc_template<__inst> > _Alloc_type;
  typedef __allocator<_Tp, __malloc_alloc_template<__inst> > allocator_type;
};

template <class _Tp, class _Tp1, bool __thr, int __inst>
struct _Alloc_traits<_Tp, 
                      __allocator<_Tp1, 
                                  __default_alloc_template<__thr, __inst> > >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, __default_alloc_template<__thr,__inst> > 
          _Alloc_type;
  typedef __allocator<_Tp, __default_alloc_template<__thr,__inst> > 
          allocator_type;
};

template <class _Tp, class _Tp1, class _Alloc>
struct _Alloc_traits<_Tp, __allocator<_Tp1, debug_alloc<_Alloc> > >
{
  static const bool _S_instanceless = true;
  typedef simple_alloc<_Tp, debug_alloc<_Alloc> > _Alloc_type;
  typedef __allocator<_Tp, debug_alloc<_Alloc> > allocator_type;
};


#endif /* __STL_USE_STD_ALLOCATORS */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#endif

__STL_END_NAMESPACE

#undef __PRIVATE

#endif /* __SGI_STL_INTERNAL_ALLOC_H */

// Local Variables:
// mode:C++
// End:
