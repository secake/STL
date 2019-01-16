/*这是堆的头文件，堆并非STL标准，是SGI 优先队列的底层结构，而堆的底层结构默认是vector
 *堆是一棵完全二叉树，堆分为大根堆和小根堆，SGI STL里的heap默认为大根堆
 */

/* 这个文件只会被SGI STL使用，不建议个人使用
 */

#ifndef __SGI_STL_INTERNAL_HEAP_H
#define __SGI_STL_INTERNAL_HEAP_H

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1209
#endif

// 堆的主要函数 push_heap, pop_heap, make_heap, sort_heap.

//插入元素到大根堆，因为要保证max_heap的结构，所以要进行上溯操作
template <class _RandomAccessIterator, class _Distance, class _Tp>
void 
__push_heap(_RandomAccessIterator __first,
            _Distance __holeIndex, _Distance __topIndex, _Tp __value)
{
	//在此操作前，要已经把元素插入到尾节点的位置

	//找到尾节点的父节点，现在把尾节点当作洞
  _Distance __parent = (__holeIndex - 1) / 2;
	//如果没有上溯到根节点，或者父节点的值小于要插入元素的值，
	//就把父节点的值下移到洞中，并把洞移到父节点
  while (__holeIndex > __topIndex && *(__first + __parent) < __value) {
    *(__first + __holeIndex) = *(__first + __parent);
    __holeIndex = __parent;
    __parent = (__holeIndex - 1) / 2;
  }    
	//直到找到了要插入的位置，就把新插入的节点值赋值给新洞
  *(__first + __holeIndex) = __value;
}

template <class _RandomAccessIterator, class _Distance, class _Tp>
inline void 
__push_heap_aux(_RandomAccessIterator __first,
                _RandomAccessIterator __last, _Distance*, _Tp*)
{
	//简单调用上面的__push_heap()
	//__first,				头部指针，迭代器都对，毕竟vector
	//_Distance((__last - __first)-1),	新插入元素的位置，第一个洞的位置
	//_Distance（0），			根节点的位置
	//_Tp(*(__last -1));,			要插入的值
  __push_heap(__first, _Distance((__last - __first) - 1), _Distance(0), 
              _Tp(*(__last - 1)));
}


//留给外部的接口，SGI STL中函数名前无__基本都是给外部用的
template <class _RandomAccessIterator>
inline void 
push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
	//不知道干了些什么，我也只看到了随机访问迭代器，用的vim，没有设置跟踪
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);

	//简单调用内部接口实现
	//前两个参数传递了范围，
	//后两个参数传递了类型，只是类型
  __push_heap_aux(__first, __last,
                  __DISTANCE_TYPE(__first), __VALUE_TYPE(__first));
}

//__push_heap指定比较方式版
//用户可以指定要比较的方式 __comp
template <class _RandomAccessIterator, class _Distance, class _Tp, 
          class _Compare>
void
__push_heap(_RandomAccessIterator __first, _Distance __holeIndex,
            _Distance __topIndex, _Tp __value, _Compare __comp)
{
  _Distance __parent = (__holeIndex - 1) / 2;
	//只有这里和第一个版本不同__comp(*(__first + __parent),value)
	//用户可以指定一个比较方式
  while (__holeIndex > __topIndex && __comp(*(__first + __parent), __value)) {
    *(__first + __holeIndex) = *(__first + __parent);
    __holeIndex = __parent;
    __parent = (__holeIndex - 1) / 2;
  }
  *(__first + __holeIndex) = __value;
}

//指定比较方式版
template <class _RandomAccessIterator, class _Compare,
          class _Distance, class _Tp>
inline void 
__push_heap_aux(_RandomAccessIterator __first,
                _RandomAccessIterator __last, _Compare __comp,
                _Distance*, _Tp*) 
{
  __push_heap(__first, _Distance((__last - __first) - 1), _Distance(0), 
              _Tp(*(__last - 1)), __comp);
}

//指定比较方式版
//不知道为什么不给 __comp一个默认值，里面判断一下就好了，为什么要写两遍？
template <class _RandomAccessIterator, class _Compare>
inline void 
push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
          _Compare __comp)
{
	//和上面那个比少了一句话呢，反正我也不直到说了啥
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);

  __push_heap_aux(__first, __last, __comp,
                  __DISTANCE_TYPE(__first), __VALUE_TYPE(__first));
}

//把__holeIndex为首的堆构成heap
template <class _RandomAccessIterator, class _Distance, class _Tp>
void 
__adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex,
              _Distance __len, _Tp __value)
{
	//找到指定树的根
  _Distance __topIndex = __holeIndex;
	//找到他的右儿子
  _Distance __secondChild = 2 * __holeIndex + 2;
 
	//直到走到尾 
 while (__secondChild < __len) {
	//把__secondChild定位到两个儿子中最大的一个 最大？WTF
    if (*(__first + __secondChild) < *(__first + (__secondChild - 1)))
      __secondChild--;
	//找到两个子节点中最大的，覆盖父节点
    *(__first + __holeIndex) = *(__first + __secondChild);
	//往后递归
    __holeIndex = __secondChild;
    __secondChild = 2 * (__secondChild + 1);
  }

	//走到最后，刚好儿子就是尾节点的话，把尾节点前面的值给爸爸，因为尾节点的值在__pop_heap中赋予根得到值，并且最大
	//并且把洞放跳到尾节点的前面
  if (__secondChild == __len) {
    *(__first + __holeIndex) = *(__first + (__secondChild - 1));
    __holeIndex = __secondChild - 1;
  }
	//最后把尾节点前面的那个节点跳到合适的位置
	//总之这里看侯捷的《STL源码剖析》很蒙，看了代码还是很蒙，等会试验下
  __push_heap(__first, __holeIndex, __topIndex, __value);
}


template <class _RandomAccessIterator, class _Tp, class _Distance>
inline void 
__pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
           _RandomAccessIterator __result, _Tp __value, _Distance*)
{
	//把根节点的值给尾节点。
	//对没错。现在尾节点的值是最大的,pop_heap结束，也是尾节点是最大的，尾节点并没有删除，后续需要用户删除
  *__result = *__first;
	//对heap进行下溯操作，相当于不考虑尾节点的情况下，把节点进行上移
	//过程解释：
	//找到两个孩子最大的一个，放到父亲的位置，
	//把最大的那个孩子当父亲，重复上面的过程
	//直到尾
  __adjust_heap(__first, _Distance(0), _Distance(__last - __first), __value);
}

//改变接口，简单调用
template <class _RandomAccessIterator, class _Tp>
inline void 
__pop_heap_aux(_RandomAccessIterator __first, _RandomAccessIterator __last,
               _Tp*)
{
  __pop_heap(__first, __last - 1, __last - 1, 
             _Tp(*(__last - 1)), __DISTANCE_TYPE(__first));
}

//对外的pop_heap操作，简单调用
template <class _RandomAccessIterator>
inline void pop_heap(_RandomAccessIterator __first, 
                     _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  __pop_heap_aux(__first, __last, __VALUE_TYPE(__first));
}


//用户可以指定比较方式版，不多balabala
template <class _RandomAccessIterator, class _Distance,
          class _Tp, class _Compare>
void
__adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex,
              _Distance __len, _Tp __value, _Compare __comp)
{
  _Distance __topIndex = __holeIndex;
  _Distance __secondChild = 2 * __holeIndex + 2;
  while (__secondChild < __len) {
    if (__comp(*(__first + __secondChild), *(__first + (__secondChild - 1))))
      __secondChild--;
    *(__first + __holeIndex) = *(__first + __secondChild);
    __holeIndex = __secondChild;
    __secondChild = 2 * (__secondChild + 1);
  }
  if (__secondChild == __len) {
    *(__first + __holeIndex) = *(__first + (__secondChild - 1));
    __holeIndex = __secondChild - 1;
  }
  __push_heap(__first, __holeIndex, __topIndex, __value, __comp);
}


//不多balabala +1
template <class _RandomAccessIterator, class _Tp, class _Compare, 
          class _Distance>
inline void 
__pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
           _RandomAccessIterator __result, _Tp __value, _Compare __comp,
           _Distance*)
{
  *__result = *__first;
  __adjust_heap(__first, _Distance(0), _Distance(__last - __first), 
                __value, __comp);
}

//不多balabala +1
template <class _RandomAccessIterator, class _Tp, class _Compare>
inline void 
__pop_heap_aux(_RandomAccessIterator __first,
               _RandomAccessIterator __last, _Tp*, _Compare __comp)
{
  __pop_heap(__first, __last - 1, __last - 1, _Tp(*(__last - 1)), __comp,
             __DISTANCE_TYPE(__first));
}

//还是和push_heap一样的话，他为什么不给__comp赋个初值呢，里面进行判断下就好了，为什么要重复写这么多代码呢
template <class _RandomAccessIterator, class _Compare>
inline void 
pop_heap(_RandomAccessIterator __first,
         _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __pop_heap_aux(__first, __last, __VALUE_TYPE(__first), __comp);
}

//用于把一段vector array 等等等变为heap
template <class _RandomAccessIterator, class _Tp, class _Distance>
void 
__make_heap(_RandomAccessIterator __first,
            _RandomAccessIterator __last, _Tp*, _Distance*)
{
	//如果就0或者1个元素，那还排个啥？再见
  if (__last - __first < 2) return;
	//长度，即元素个数
  _Distance __len = __last - __first;
	//找到最后一个父节点，因为所有叶子节点根本不影响heap的结构
  _Distance __parent = (__len - 2)/2;
    
  while (true) {
	//这里对每个爸爸为根的树 来进行排列成heap
	//其实可以只判断他的两个孩子的是否有比他大的就好了，没必要每个爸爸为首的树都要生成
    __adjust_heap(__first, __parent, __len, _Tp(*(__first + __parent)));
	//遍历所有爸爸  
  if (__parent == 0) return;
    __parent--;
  }
}

//外部接口，调用内部函数
template <class _RandomAccessIterator>
inline void 
make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  __make_heap(__first, __last,
              __VALUE_TYPE(__first), __DISTANCE_TYPE(__first));
}

//用户自定义比较方式版，没有更多区别
template <class _RandomAccessIterator, class _Compare,
          class _Tp, class _Distance>
void
__make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
            _Compare __comp, _Tp*, _Distance*)
{
  if (__last - __first < 2) return;
  _Distance __len = __last - __first;
  _Distance __parent = (__len - 2)/2;
    
  while (true) {
    __adjust_heap(__first, __parent, __len, _Tp(*(__first + __parent)),
                  __comp);
    if (__parent == 0) return;
    __parent--;
  }
}

//用户自定义比较方式版，没有更多区别
template <class _RandomAccessIterator, class _Compare>
inline void 
make_heap(_RandomAccessIterator __first, 
          _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __make_heap(__first, __last, __comp,
              __VALUE_TYPE(__first), __DISTANCE_TYPE(__first));
}

//对堆进行排序，排序后为从小到大的序列，也是一个特殊的小根堆
template <class _RandomAccessIterator>
void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
	//因为调用pop_heap后，尾节点便是最大的节点，所以每次去掉尾节点来调用pop_heap后，heap即递增了
	//虽然看起来算法好像有点粗鲁，但是时间复杂度才O（nlgn）有木有
  while (__last - __first > 1)
    pop_heap(__first, __last--);
}

//用户自定义比较方式版，如果__comp是less，所得序列是递减
template <class _RandomAccessIterator, class _Compare>
void 
sort_heap(_RandomAccessIterator __first,
          _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  while (__last - __first > 1)
    pop_heap(__first, __last--, __comp);
}

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1209
#endif

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_HEAP_H */

// Local Variables:
// mode:C++
// End:
