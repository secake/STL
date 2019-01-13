//STL多数文件都有此声明，表示这是SGI的文档，且通常情况下之后stl会进行调用
/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_STL_INTERNAL_ITERATOR_BASE_H
#define __SGI_STL_INTERNAL_ITERATOR_BASE_H


#include <concept_checks.h>

__STL_BEGIN_NAMESPACE

//五种迭代器
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};



//这个文件定义了iterator_traits，外界可以调用iterator_traits<int>::value_type等形式来获取一个变量的类型
//提供了一种随时来获取迭代器类型的方式，迭代器共定义了五种类型，如上面的五种，这是c++标准
//五种类型分别如下

//这个input_iterator表示 这个表示此类型迭代器只能进行读操作
template <class _Tp, class _Distance> struct input_iterator {
  typedef input_iterator_tag iterator_category;	//这个就是迭代器的类型，即五种类型
  typedef _Tp                value_type;	//这个表示迭代器所指向变量的值类型
  typedef _Distance          difference_type;	//这个表示迭代器所指向容器所能表示范围，即相当于size_t/sizeof(_Tp)
  typedef _Tp*               pointer;		//这个表示迭代器所指向变量的指针
  typedef _Tp&               reference;		//这个表示迭代器所指向变量的引用
};

//这个output_iterator表示，这个类型的迭代器只能进行写操作
struct output_iterator {
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;
};

//这个类型的迭代器只能进行前向访问
template <class _Tp, class _Distance> struct forward_iterator {
  typedef forward_iterator_tag iterator_category;
  typedef _Tp                  value_type;
  typedef _Distance            difference_type;
  typedef _Tp*                 pointer;
  typedef _Tp&                 reference;
};


//这个类型的迭代器可以进行双向访问
template <class _Tp, class _Distance> struct bidirectional_iterator {
  typedef bidirectional_iterator_tag iterator_category;
  typedef _Tp                        value_type;
  typedef _Distance                  difference_type;
  typedef _Tp*                       pointer;
  typedef _Tp&                       reference;
};

//这是最高级迭代器，可以进行所有指针的操作
template <class _Tp, class _Distance> struct random_access_iterator {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                        value_type;
  typedef _Distance                  difference_type;
  typedef _Tp*                       pointer;
  typedef _Tp&                       reference;
};


//这个是stl的标准定义，表示只要继承此类的迭代器就符合stl标准
//而且只需要实现前两个变量的定义，因为后三个已经赋给了默认值
#ifdef __STL_USE_NAMESPACES
template <class _Category, class _Tp, class _Distance = ptrdiff_t,
          class _Pointer = _Tp*, class _Reference = _Tp&>
struct iterator {
  typedef _Category  iterator_category;
  typedef _Tp        value_type;
  typedef _Distance  difference_type;
  typedef _Pointer   pointer;
  typedef _Reference reference;
};
#endif /* __STL_USE_NAMESPACES */

#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION

//缺省迭代器特化
template <class _Iterator>
struct iterator_traits {
  typedef typename _Iterator::iterator_category iterator_category;
  typedef typename _Iterator::value_type        value_type;
  typedef typename _Iterator::difference_type   difference_type;
  typedef typename _Iterator::pointer           pointer;
  typedef typename _Iterator::reference         reference;
};

//iterator_traits针对指针类型的特化
template <class _Tp>
struct iterator_traits<_Tp*> {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef _Tp*                        pointer;
  typedef _Tp&                        reference;
};


//iterator_traits针对常量指针类型的特化
template <class _Tp>
struct iterator_traits<const _Tp*> {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef const _Tp*                  pointer;
  typedef const _Tp&                  reference;
};


//SGI提供了几个iterator_traits调用的方便的接口，如下

//获取迭代器类型
template <class _Iter>
inline typename iterator_traits<_Iter>::iterator_category
__iterator_category(const _Iter&)
{
  typedef typename iterator_traits<_Iter>::iterator_category _Category;
  return _Category();
}

//获取最大值类型
template <class _Iter>
inline typename iterator_traits<_Iter>::difference_type*
__distance_type(const _Iter&)
{
  return static_cast<typename iterator_traits<_Iter>::difference_type*>(0);
}

//获取引用类型
template <class _Iter>
inline typename iterator_traits<_Iter>::value_type*
__value_type(const _Iter&)
{
  return static_cast<typename iterator_traits<_Iter>::value_type*>(0);
}

//只是单纯转调用上面三个，没搞明白为什么要分开
template <class _Iter>
inline typename iterator_traits<_Iter>::iterator_category
iterator_category(const _Iter& __i) { return __iterator_category(__i); }


template <class _Iter>
inline typename iterator_traits<_Iter>::difference_type*
distance_type(const _Iter& __i) { return __distance_type(__i); }

template <class _Iter>
inline typename iterator_traits<_Iter>::value_type*
value_type(const _Iter& __i) { return __value_type(__i); }

#define __ITERATOR_CATEGORY(__i) __iterator_category(__i)
#define __DISTANCE_TYPE(__i)     __distance_type(__i)
#define __VALUE_TYPE(__i)        __value_type(__i)

#else /* __STL_CLASS_PARTIAL_SPECIALIZATION */


//以下为对iterator_traits的特化
template <class _Tp, class _Distance> 
inline input_iterator_tag 
iterator_category(const input_iterator<_Tp, _Distance>&)
  { return input_iterator_tag(); }

inline output_iterator_tag iterator_category(const output_iterator&)
  { return output_iterator_tag(); }

template <class _Tp, class _Distance> 
inline forward_iterator_tag
iterator_category(const forward_iterator<_Tp, _Distance>&)
  { return forward_iterator_tag(); }

template <class _Tp, class _Distance> 
inline bidirectional_iterator_tag
iterator_category(const bidirectional_iterator<_Tp, _Distance>&)
  { return bidirectional_iterator_tag(); }

template <class _Tp, class _Distance> 
inline random_access_iterator_tag
iterator_category(const random_access_iterator<_Tp, _Distance>&)
  { return random_access_iterator_tag(); }

template <class _Tp>
inline random_access_iterator_tag iterator_category(const _Tp*)
  { return random_access_iterator_tag(); }

template <class _Tp, class _Distance> 
inline _Tp* value_type(const input_iterator<_Tp, _Distance>&)
  { return (_Tp*)(0); }

template <class _Tp, class _Distance> 
inline _Tp* value_type(const forward_iterator<_Tp, _Distance>&)
  { return (_Tp*)(0); }

template <class _Tp, class _Distance> 
inline _Tp* value_type(const bidirectional_iterator<_Tp, _Distance>&)
  { return (_Tp*)(0); }

template <class _Tp, class _Distance> 
inline _Tp* value_type(const random_access_iterator<_Tp, _Distance>&)
  { return (_Tp*)(0); }

template <class _Tp>
inline _Tp* value_type(const _Tp*) { return (_Tp*)(0); }

template <class _Tp, class _Distance> 
inline _Distance* distance_type(const input_iterator<_Tp, _Distance>&)
{
  return (_Distance*)(0);
}

template <class _Tp, class _Distance> 
inline _Distance* distance_type(const forward_iterator<_Tp, _Distance>&)
{
  return (_Distance*)(0);
}

template <class _Tp, class _Distance> 
inline _Distance* 
distance_type(const bidirectional_iterator<_Tp, _Distance>&)
{
  return (_Distance*)(0);
}

template <class _Tp, class _Distance> 
inline _Distance* 
distance_type(const random_access_iterator<_Tp, _Distance>&)
{
  return (_Distance*)(0);
}

template <class _Tp>
inline ptrdiff_t* distance_type(const _Tp*) { return (ptrdiff_t*)(0); }

// Without partial specialization we can't use iterator_traits, so
// we must keep the old iterator query functions around.  

#define __ITERATOR_CATEGORY(__i) iterator_category(__i)
#define __DISTANCE_TYPE(__i)     distance_type(__i)
#define __VALUE_TYPE(__i)        value_type(__i)

#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

//对distance的实现，判断两个迭代器之间的距离，利用了iterator_traits的特性，注意，对input_iterator和random_access_iterator的实现不同
//所以后者的复杂度为O(1)，前者的复杂度为O(n)
template <class _InputIterator, class _Distance>
inline void __distance(_InputIterator __first, _InputIterator __last,
                       _Distance& __n, input_iterator_tag)
{
  while (__first != __last) { ++__first; ++__n; }
}

template <class _RandomAccessIterator, class _Distance>
inline void __distance(_RandomAccessIterator __first, 
                       _RandomAccessIterator __last, 
                       _Distance& __n, random_access_iterator_tag)
{
  __STL_REQUIRES(_RandomAccessIterator, _RandomAccessIterator);
  __n += __last - __first;
}
//对外接口
template <class _InputIterator, class _Distance>
inline void distance(_InputIterator __first, 
                     _InputIterator __last, _Distance& __n)
{
  __STL_REQUIRES(_InputIterator, _InputIterator);
	//iterator_category调用了Iterator_traits的特性
  __distance(__first, __last, __n, iterator_category(__first));
}

#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
//和上面作用相同
template <class _InputIterator>
inline typename iterator_traits<_InputIterator>::difference_type
__distance(_InputIterator __first, _InputIterator __last, input_iterator_tag)
{
  typename iterator_traits<_InputIterator>::difference_type __n = 0;
  while (__first != __last) {
    ++__first; ++__n;
  }
  return __n;
}

template <class _RandomAccessIterator>
inline typename iterator_traits<_RandomAccessIterator>::difference_type
__distance(_RandomAccessIterator __first, _RandomAccessIterator __last,
           random_access_iterator_tag) {
  __STL_REQUIRES(_RandomAccessIterator, _RandomAccessIterator);
  return __last - __first;
}

template <class _InputIterator>
inline typename iterator_traits<_InputIterator>::difference_type
distance(_InputIterator __first, _InputIterator __last) {
  typedef typename iterator_traits<_InputIterator>::iterator_category 
    _Category;
  __STL_REQUIRES(_InputIterator, _InputIterator);
  return __distance(__first, __last, _Category());
}

#endif /* __STL_CLASS_PARTIAL_SPECIALIZATION */

//对advance的实现，用于将一个迭代器向__n方向移动__n距离，
//当然对于_RandomAccessITerator的实现效率最高，为O(1)的复杂度
template <class _InputIter, class _Distance>
inline void __advance(_InputIter& __i, _Distance __n, input_iterator_tag) {
  while (__n--) ++__i;
}

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1183
#endif

template <class _BidirectionalIterator, class _Distance>
inline void __advance(_BidirectionalIterator& __i, _Distance __n, 
                      bidirectional_iterator_tag) {
  __STL_REQUIRES(_BidirectionalIterator, _BidirectionalIterator);
  if (__n >= 0)
    while (__n--) ++__i;
  else
    while (__n++) --__i;
}

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1183
#endif

template <class _RandomAccessIterator, class _Distance>
inline void __advance(_RandomAccessIterator& __i, _Distance __n, 
                      random_access_iterator_tag) {
  __STL_REQUIRES(_RandomAccessIterator, _RandomAccessIterator);
  __i += __n;
}

template <class _InputIterator, class _Distance>
inline void advance(_InputIterator& __i, _Distance __n) {
  __STL_REQUIRES(_InputIterator, _InputIterator);
  __advance(__i, __n, iterator_category(__i));
}

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_ITERATOR_BASE_H */



// Local Variables:
// mode:C++
// End:
