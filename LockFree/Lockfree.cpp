//
// Game Programming Gems 6
// Lock free multithreaded algorithms
// By Toby Jones
// Supports Microsoft Visual C++ and GCC.
//

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>

#include <windows.h>
#include <process.h>

#ifdef _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant
#endif

//------------------------------------------------------------------------------
//
// Definitions of CAS.
//
//------------------------------------------------------------------------------

//
// Define what code we will be using for CAS.
// Intrinsic only available on Visual C++.
//
#ifndef CAS
#ifdef _X86_
#define CAS CAS_assembly
#elif defined(__GNUC__)
#define CAS CAS_windows
#elif defined(_MSC_VER)
#define CAS CAS_intrinsic
#else
#error No CAS implemented for this compiler/architecture combination.
#endif
#endif

// this function is atomic
// bool CAS(uint32_t *ptr, uint32_t oldVal, uint32_t newVal)
// {
//     if(*ptr == oldVal)
//     {
//         *ptr = newVal;
//         return true;
//     }
//     return false;
// }

//
// All of the CAS functions will operate on a node.  So we define node first.
//
template<typename Ty>
struct node
{
    Ty value;
    node<Ty> * volatile pNext;

    node() : value(), pNext(nullptr) {}
    node(Ty v) : pNext(nullptr), value(v) {}
};

// CAS will assume a multi-processor machine (versus multithread on a single processor).
// On a single processor machine, it might not make sense to spin on a CAS because
// if it fails, there is no way to succeed until the another thread runs (which will be
// on the same processor).  In these cases, if CAS is required to succeed, it might make
// more sense to yield the processor to the other thread via Sleep(1).

// Since we are assuming a multi-processor machine, we will need to use 'lock cmpxchg'
// instead of just cmpxchg, as the bus needs to be locked to synchronize the processors
// access to memory.

// SAL annotations on most parameters in CAS/CAS2 are not necessary, as the pointers
// themselves are not dereferenced.

//
// Define a version of CAS which uses x86 assembly primitives.
//
#ifdef _X86_
template<typename Ty>
bool CAS_assembly(_Inout_ node<Ty> * volatile * _ptr, node<Ty> * oldVal, node<Ty> * newVal)
{
    register bool f;

#ifdef __GNUC__
    __asm__ __volatile__(
        "lock; cmpxchgl %%ebx, %1;"
        "setz %0;"
            : "=r"(f), "=m"(*(_ptr))
            : "a"(oldVal), "b" (newVal)
            : "memory");
#else
    _asm
    {
        mov ecx,_ptr
        mov eax,oldVal
        mov ebx,newVal
        lock cmpxchg [ecx],ebx
        setz f
    }
#endif

    return f;
}
#endif // _X86_

//
// Define a version of CAS which uses the Visual C++ InterlockedCompareExchange intrinsic.
//
#ifdef _MSC_VER

// Define intrinsic for InterlockedCompareExchange
extern "C" long __cdecl _InterlockedCompareExchange(_Inout_ long volatile * Dest, long Exchange, long Comp);

#pragma intrinsic(_InterlockedCompareExchange)

template<typename Ty>
bool CAS_intrinsic(_Inout_ node<Ty> * volatile * _ptr, node<Ty> * oldVal, node<Ty> * newVal)
{
    return _InterlockedCompareExchange((long *)_ptr, (long)newVal, (long)oldVal) == (long)oldVal;
}

#endif  // _MSC_VER

//
// Define a version of CAS which uses the Windows API InterlockedCompareExchange.
//
template<typename Ty>
bool CAS_windows(_Inout_ node<Ty> * volatile * _ptr, node<Ty> * oldVal, node<Ty> * newVal)
{
    return InterlockedCompareExchange((long *)_ptr, (long)newVal, (long)oldVal) == (long)oldVal;
}

//------------------------------------------------------------------------------
//
// Definitions of CAS2.
//
//------------------------------------------------------------------------------

//
// Define what code we will be using for CAS2.
// Intrinsic only available on Visual C++.
// Windows version only available on Windows Vista.
//
#ifndef CAS2
#ifdef _X86_
#define CAS2 CAS2_assembly
#elif defined(__GNUC__)
#define CAS2 CAS2_windows
#elif defined(_MSC_VER)
#define CAS2 CAS2_intrinsic
#else
#error No CAS2 implemented for this compiler/architecture combination.
#endif
#endif

// _Inout_count_c_(2) is the best SAL annotation possible, though it will not
// prevent a single element buffer from being passed.

//
// Define a version of CAS2 which uses x86 assembly primitives.
//
#ifdef _X86_
template<typename Ty>
bool CAS2_assembly(_Inout_count_c_(2) node<Ty> * volatile * _ptr, node<Ty> * old1, uint32_t old2, node<Ty> * new1, uint32_t new2)
{
    register bool f;
#ifdef __GNUC__
    __asm__ __volatile__(
        "lock; cmpxchg8b %1;"
        "setz %0;"
            : "=r"(f), "=m"(*(_ptr))
            : "a"(old1), "b" (new1), "c" (new2), "d" (old2)
            : "memory");
#else
    _asm
    {
        mov esi,_ptr
        mov eax,old1
        mov edx,old2
        mov ebx,new1
        mov ecx,new2
        lock cmpxchg8b [esi]
        setz f
    }
#endif
    return f;
}
#endif // _X86_

//
// Define a version of CAS2 which uses the Visual C++ InterlockedCompareExchange64 intrinsic.
//
#ifdef _MSC_VER

// Define intrinsic for InterlockedCompareExchange64
extern "C" __int64 __cdecl _InterlockedCompareExchange64(_Inout_ __int64 volatile * Destination, __int64 Exchange, __int64 Comperand);

#pragma intrinsic(_InterlockedCompareExchange64)

template<typename Ty>
bool CAS2_intrinsic(_Inout_count_c_(2) node<Ty> * volatile * _ptr, node<Ty> * old1, uint32_t old2, node<Ty> * new1, uint32_t new2)
{
    LONGLONG Comperand = reinterpret_cast<LONG>(old1) | (static_cast<LONGLONG>(old2) << 32);
    LONGLONG Exchange  = reinterpret_cast<LONG>(new1) | (static_cast<LONGLONG>(new2) << 32);

    return _InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile *>(_ptr), Exchange, Comperand) == Comperand;
}

#endif  // _MSC_VER

//
// Define a version of CAS2 which uses the Windows API InterlockedCompareExchange64.
// InterlockedCompareExchange64 requires Windows Vista
//
#if WINVER >= 0x0600

// LONGLONG InterlockedCompareExchange64(LONGLONG volatile * Destination, LONGLONG Exchange, LONGLONG Comperand);

template<typename Ty>
bool CAS2_windows(_Inout_count_c_(2) node<Ty> * volatile * _ptr, node<Ty> * old1, uint32_t old2, node<Ty> * new1, uint32_t new2)
{
    LONGLONG Comperand = reinterpret_cast<long>(old1) | (static_cast<LONGLONG>(old2) << 32);
    LONGLONG Exchange  = reinterpret_cast<long>(new1) | (static_cast<LONGLONG>(new2) << 32);

    return InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile *>(_ptr), Exchange, Comperand) == Comperand;
}
#endif  // WINVER >= 0x0600

//------------------------------------------------------------------------------
//
// Parameterized Lock-free Stack
//
//------------------------------------------------------------------------------
template<typename Ty>
class LockFreeStack
{
    // NOTE: the order of these members is assumed by CAS2.
    node<Ty> * volatile _pHead;
    volatile uint32_t  _cPops;

public:
    // The correct SAL annotation is _In_, but /analyze cannot deduce the
    // size of node<Ty> unless it is made explicit.
    void Push(_In_bytecount_c_(sizeof node<Ty>) node<Ty> * pNode);
    node<Ty> * Pop();

    LockFreeStack() : _pHead(nullptr), _cPops(0) {}
};

template<typename Ty>
void LockFreeStack<Ty>::Push(_In_bytecount_c_(sizeof node<Ty>) node<Ty> * pNode)
{
    while(true)
    {
        pNode->pNext = _pHead;
        if(CAS(&_pHead, pNode->pNext, pNode))
        {
            break;
        }
    }
}

template<typename Ty>
node<Ty> * LockFreeStack<Ty>::Pop()
{
    while(true)
    {
        node<Ty> * pHead = _pHead;
        uint32_t  cPops = _cPops;
        if(nullptr == pHead)
        {
            return nullptr;
        }

        // NOTE: Memory reclaimation is difficult in this context.  If another thread breaks in here
        // and pops the head, and then frees it, then pHead->pNext is an invalid operation.  One solution
        // would be to use hazard pointers (http://researchweb.watson.ibm.com/people/m/michael/ieeetpds-2004.pdf).

        node<Ty> * pNext = pHead->pNext;
        if(CAS2(&_pHead, pHead, cPops, pNext, cPops + 1))
        {
            return pHead;
        }
    }
}

//------------------------------------------------------------------------------
//
// Parameterized Lock-free Queue
//
//------------------------------------------------------------------------------
template<typename Ty>
class LockFreeQueue {
    // NOTE: the order of these members is assumed by CAS2.
    node<Ty> * volatile _pHead;
    volatile uint32_t  _cPops;
    node<Ty> * volatile _pTail;
    volatile uint32_t  _cPushes;

public:
    // The correct SAL annotation is _In_, but /analyze cannot deduce the
    // size of node<Ty> unless it is made explicit.
    void Add(_In_bytecount_c_(sizeof node<Ty>) node<Ty> * pNode);
    node<Ty> * Remove();

    LockFreeQueue(_In_ node<Ty> * pDummy) : _cPops(0), _cPushes(0)
    {
        _pHead = _pTail = pDummy;
    }
};

template<typename Ty>
void LockFreeQueue<Ty>::Add(_In_bytecount_c_(sizeof node<Ty>) node<Ty> * pNode)
{
    pNode->pNext = nullptr;

    uint32_t cPushes;
    node<Ty> * pTail;

    while(true)
    {
        cPushes = _cPushes;
        pTail = _pTail;

        // NOTE: The Queue has the same consideration as the Stack.  If _pTail is
        // freed on a different thread, then this code can cause an access violation.

        // If the node that the tail points to is the last node
        // then update the last node to point at the new node.
        if(CAS(&(_pTail->pNext), reinterpret_cast<node<Ty> *>(nullptr), pNode))
        {
            break;
        }
        else
        {
            // Since the tail does not point at the last node,
            // need to keep updating the tail until it does.
            CAS2(&_pTail, pTail, cPushes, _pTail->pNext, cPushes + 1);
        }
    }

    // If the tail points to what we thought was the last node
    // then update the tail to point to the new node.
    CAS2(&_pTail, pTail, cPushes, pNode, cPushes + 1);
}

template<typename Ty>
node<Ty> * LockFreeQueue<Ty>::Remove()
{
    Ty value = Ty();
    node<Ty> * pHead;

    while(true)
    {
        uint32_t cPops = _cPops;
        uint32_t cPushes = _cPushes;
        pHead = _pHead;
        node<Ty> * pNext = pHead->pNext;

        // Verify that we did not get the pointers in the middle
        // of another update.
        if(cPops != _cPops)
        {
            continue;
        }
        // Check if the queue is empty.
        if(pHead == _pTail)
        {
            if(nullptr == pNext)
            {
                pHead = nullptr;    // queue is empty
                break;
            }
            // Special case if the queue has nodes but the tail
            // is just behind. Move the tail off of the head.
            CAS2(&_pTail, pHead, cPushes, pNext, cPushes + 1);
        }
        else if(nullptr != pNext)
        {
            value = pNext->value;
            // Move the head pointer, effectively removing the node
            if(CAS2(&_pHead, pHead, cPops, pNext, cPops + 1))
            {
                break;
            }
        }
    }
    if(nullptr != pHead)
    {
        pHead->value = value;
    }
    return pHead;
}

//------------------------------------------------------------------------------
//
// Parameterized Lock-free Freelist
//
//------------------------------------------------------------------------------
template<typename Ty>
class LockFreeFreeList
{
    //
    // Memory reclaimation is generally difficult with lock-free algorithms,
    // so we bypass the situation by making the object own all of the memory,
    // and creating and destroying the nodes on the thread that controls the
    // object's lifetime.  Any thread synchronization should be done at that
    // point.
    //
    LockFreeStack<Ty> _Freelist;
    node<Ty> * _pObjects;
    const uint32_t _cObjects;

    // Not implemented to prevent accidental copying.
    LockFreeFreeList(const LockFreeFreeList&);
    LockFreeFreeList& operator=(const LockFreeFreeList&);

public:
    //
    // cObjects is passed to the constructor instead of a template parameter
    // to minimize the code bloat of multiple freelists with varying sizes,
    // but each using the same underlying type.
    //
    LockFreeFreeList(uint32_t cObjects) : _cObjects(cObjects)
    {
        //
        // The Freelist may live on the stack, so we allocate the
        // actual nodes on the heap to minimize the space hit.
        //
        _pObjects = new node<Ty>[cObjects];
        FreeAll();
    }
    ~LockFreeFreeList()
    {
#ifdef _DEBUG
        for(uint32_t ix = 0; ix < _cObjects; ++ix)
        {
            assert(_Freelist.Pop() != nullptr);
        }
#endif

        delete[] _pObjects;
    }
    void FreeAll()
    {
        for(uint32_t ix = 0; ix < _cObjects; ++ix)
        {
            _Freelist.Push(&_pObjects[ix]);
        }
    }
    Ty * NewInstance()
    {
        node<Ty> * pInstance = _Freelist.Pop();
        return new(&pInstance->value) Ty;
    }
    // This is the best annotation possible given that the code hides
    // that a node structure is actually what is being passed.
    // This will not prevent an unwrapped 'Ty' from being passed.
    void FreeInstance(_In_bytecount_c_(sizeof node<Ty>) Ty * pInstance)
    {
        pInstance->~Ty();
        _Freelist.Push(reinterpret_cast<node<Ty> *>(pInstance));
    }
};

//------------------------------------------------------------------------------
//
// Test/Demo code for Lock-free Algorithms
//
//------------------------------------------------------------------------------
struct MyStruct
{
    int iValue;
    short sValue;
    char cValue;
};

typedef double TEST_TYPE;
const bool FULL_TRACE = false;

//
// Verify Assembly version of CAS.
//
void Test_CAS_assembly()
{
    std::cout << "Testing CAS_assembly...";

#ifdef _X86_
    node<MyStruct> oldVal;
    node<MyStruct> newVal;
    node<MyStruct> * pNode = &newVal;
    if(CAS_assembly(&pNode, &oldVal, &newVal))
    {
        std::cout << "CAS is INCORRECT." << std::endl;
    }
    else
    {
        pNode = &oldVal;
        if(!CAS_assembly(&pNode, &oldVal, &newVal))
        {
            std::cout << "CAS is INCORRECT." << std::endl;
        }
        else if(pNode != &newVal)
        {
            std::cout << "CAS is INCORRECT." << std::endl;
        }
        else
        {
            std::cout << "CAS is correct." << std::endl;
        }
    }
#else
    std::cout << "CAS_assembly is not implemented for this architecture." << std::endl;
#endif
}

//
// Verify compiler intrinsic version of CAS.
//
void Test_CAS_intrinsic()
{
    std::cout << "Testing CAS_intrinsic...";

#ifdef _MSC_VER
    node<MyStruct> oldVal;
    node<MyStruct> newVal;
    node<MyStruct> * pNode = &newVal;
    if(CAS_intrinsic(&pNode, &oldVal, &newVal))
    {
        std::cout << "CAS is INCORRECT." << std::endl;
    }
    else
    {
        pNode = &oldVal;
        if(!CAS_intrinsic(&pNode, &oldVal, &newVal))
        {
            std::cout << "CAS is INCORRECT." << std::endl;
        }
        else if(pNode != &newVal)
        {
            std::cout << "CAS is INCORRECT." << std::endl;
        }
        else
        {
            std::cout << "CAS is correct." << std::endl;
        }
    }
#else
    std::cout << "CAS_intrinsic is not implemented for this compiler." << std::endl;
#endif
}

//
// Verify Windows API version of CAS.
//
void Test_CAS_windows()
{
    std::cout << "Testing CAS_windows...";

    node<MyStruct> oldVal;
    node<MyStruct> newVal;
    node<MyStruct> * pNode = &newVal;
    if(CAS_windows(&pNode, &oldVal, &newVal))
    {
        std::cout << "CAS is INCORRECT." << std::endl;
    }
    else
    {
        pNode = &oldVal;
        if(!CAS_windows(&pNode, &oldVal, &newVal))
        {
            std::cout << "CAS is INCORRECT." << std::endl;
        }
        else if(pNode != &newVal)
        {
            std::cout << "CAS is INCORRECT." << std::endl;
        }
        else
        {
            std::cout << "CAS is correct." << std::endl;
        }
    }
}

template<typename Ty>
struct CAS2Test
{
    node<Ty> * pNode;
    uint32_t tag;
    CAS2Test(_In_ node<Ty> * pnewNode, uint32_t newTag) : pNode(pnewNode), tag(newTag) {}
};

//
// Verify Assembly version of CAS.
//
void Test_CAS2_assembly()
{
    std::cout << "Testing CAS2_assembly...";

#ifdef _X86_
    node<MyStruct> oldVal;
    node<MyStruct> newVal;

    CAS2Test<MyStruct> myStruct(&newVal, 0xABCD);

    if(CAS2_assembly(&myStruct.pNode, &oldVal, 0xABCD, &newVal, 0xAAAA))
    {
        // should not succeed if pointers don't match
        std::cout << "CAS2 is INCORRECT." << std::endl;
    }
    else if(CAS2_assembly(&myStruct.pNode, &newVal, 0xAAAA, &oldVal, 0xABCD))
    {
        // should not succeed if tags don't match
        std::cout << "CAS2 is INCORRECT." << std::endl;
    }
    else
    {
        myStruct.pNode = &oldVal;
        if(!CAS2_assembly(&myStruct.pNode, &oldVal, 0xABCD, &newVal, 0xAAAA))
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else if(myStruct.pNode != &newVal)
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else if(myStruct.tag != 0xAAAA)
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else
        {
            std::cout << "CAS2 is correct." << std::endl;
        }
    }
#else
    std::cout << "CAS2_assembly not implemented for this architecture." << std::endl;
#endif
}

//
// Verify compiler intrinsic version of CAS.
//
void Test_CAS2_intrinsic()
{
    std::cout << "Testing CAS2_intrinsic...";

#ifdef _MSC_VER
    node<MyStruct> oldVal;
    node<MyStruct> newVal;

    CAS2Test<MyStruct> myStruct(&newVal, 0xABCD);

    if(CAS2_intrinsic(&myStruct.pNode, &oldVal, 0xABCD, &newVal, 0xAAAA))
    {
        // should not succeed if pointers don't match
        std::cout << "CAS2 is INCORRECT." << std::endl;
    }
    else if(CAS2_intrinsic(&myStruct.pNode, &newVal, 0xAAAA, &oldVal, 0xABCD))
    {
        // should not succeed if tags don't match
        std::cout << "CAS2 is INCORRECT." << std::endl;
    }
    else
    {
        myStruct.pNode = &oldVal;
        if(!CAS2_intrinsic(&myStruct.pNode, &oldVal, 0xABCD, &newVal, 0xAAAA))
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else if(myStruct.pNode != &newVal)
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else if(myStruct.tag != 0xAAAA)
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else
        {
            std::cout << "CAS2 is correct." << std::endl;
        }
    }
#else
    std::cout << "CAS2_intrinsic is not implemented for this compiler." << std::endl;
#endif
}

//
// Verify Windows API version of CAS2.
//
void Test_CAS2_windows()
{
    std::cout << "Testing CAS2_windows...";

#if WINVER >= 0x0600
    node<MyStruct> oldVal;
    node<MyStruct> newVal;

    CAS2Test<MyStruct> myStruct(&newVal, 0xABCD);

    if(CAS2_windows(&myStruct.pNode, &oldVal, 0xABCD, &newVal, 0xAAAA))
    {
        // should not succeed if pointers don't match
        std::cout << "CAS2 is INCORRECT." << std::endl;
    }
    else if(CAS2_windows(&myStruct.pNode, &newVal, 0xAAAA, &oldVal, 0xABCD))
    {
        // should not succeed if tags don't match
        std::cout << "CAS2 is INCORRECT." << std::endl;
    }
    else
    {
        myStruct.pNode = &oldVal;
        if(!CAS2_windows(&myStruct.pNode, &oldVal, 0xABCD, &newVal, 0xAAAA))
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else if(myStruct.pNode != &newVal)
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else if(myStruct.tag != 0xAAAA)
        {
            std::cout << "CAS2 is INCORRECT." << std::endl;
        }
        else
        {
            std::cout << "CAS2 is correct." << std::endl;
        }
    }
#else
    std::cout << "CAS2_windows is not implemented on this architecture." << std::endl;
#endif
}

void HandleWait(HANDLE & hThread)
{
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}

template<typename Ty>
void CreateNode(node<Ty> * & pNode)
{
    pNode = new node<Ty>;
}

template<typename Ty>
void DeleteNode(node<Ty> * & pNode)
{
    delete pNode;
}

//
// Stress the multithreaded stack code.
//
template<typename Ty, int NUMTHREADS>
class StressStack
{
    LockFreeStack<Ty> _stack;

    static const unsigned int cNodes = 100;    // nodes per thread

    struct ThreadData
    {
        StressStack<Ty, NUMTHREADS> * pStress;
        DWORD thread_num;
    };

    std::vector<ThreadData> _aThreadData;
    std::vector<node<Ty> *> _apNodes;

public:
    StressStack() : _aThreadData(NUMTHREADS), _apNodes(cNodes * NUMTHREADS) {}

    //
    // The stack stress will spawn a number of threads (4096 in our tests), each of which will
    // push and pop nodes onto a single stack.  We expect that no access violations will occur
    // and that the stack is empty upon completion.
    //
    void operator()()
    {
        std::cout << "Running Stack Stress..." << std::endl;

        //
        // Create all of the nodes.
        //
        std::for_each(_apNodes.begin(), _apNodes.end(), CreateNode<Ty>);

        unsigned int ii;
        for(ii = 0; ii < _aThreadData.size(); ++ii)
        {
            _aThreadData[ii].pStress = this;
            _aThreadData[ii].thread_num = ii;
        }

        std::vector<HANDLE> aHandles(NUMTHREADS);
        for(ii = 0; ii < aHandles.size(); ++ii)
        {
            unsigned int tid;
            aHandles[ii] = (HANDLE)_beginthreadex(nullptr, 0, StackThreadFunc, &_aThreadData[ii], 0, &tid);
        }

        //
        // Wait for the threads to exit.
        //
        std::for_each(aHandles.begin(), aHandles.end(), HandleWait);

        //
        // Delete all of the nodes.
        //
        std::for_each(_apNodes.begin(), _apNodes.end(), DeleteNode<Ty>);

        //
        // Ideas for improvement:
        //  We could verify that there is a 1-1 mapping between values pushed and values popped.
        //  Verify the count of pops in the stack matches the number of pops for each thread.
        //
    } // void operator()()

    static unsigned int __stdcall StackThreadFunc(_In_ void * pv)
    {
        unsigned int tid = GetCurrentThreadId();
        ThreadData * ptd = reinterpret_cast<ThreadData *>(pv);
        if(FULL_TRACE)
        {
            std::cout << tid << " adding" << std::endl;
        }

        unsigned int ii;
        for(ii = 0; ii < cNodes; ++ii)
        {
            ptd->pStress->_stack.Push(ptd->pStress->_apNodes[ptd->thread_num * cNodes + ii]);
        }

        if(FULL_TRACE)
        {
            std::cout << tid << " removing" << std::endl;
        }

        for(ii = 0; ii < cNodes; ++ii)
        {
            ptd->pStress->_apNodes[ptd->thread_num * cNodes + ii] = ptd->pStress->_stack.Pop();
        }

        return 0;
    }
};  // class StressStack

//
// Stress the multithreaded queue code.
//
template<typename Ty, int NUMTHREADS>
class StressQueue
{
    LockFreeQueue<Ty> _queue;

    struct ThreadData
    {
        StressQueue<Ty, NUMTHREADS> * pStress;
        DWORD thread_num;
    };

    std::vector<ThreadData> _aThreadData;
    std::vector<node<Ty> *> & _apNodes;

    // Declare undefined assignment operator due to const data member.
    // http://support.microsoft.com/kb/87638
    StressQueue& operator=(const StressQueue&);

public:
    static const unsigned int cNodes = 100;     // nodes per thread

    StressQueue(std::vector<node<Ty> *> & apNodes) : _queue(apNodes[0]), _aThreadData(NUMTHREADS), _apNodes(apNodes) {}

    //
    // The queue stress will spawn a number of threads (4096 in our tests), each of which will
    // add and remove nodes on a single queue.  We expect that no access violations will occur
    // and that the queue is empty (except for the dummy node) upon completion.
    //
    void operator()()
    {
        std::cout << "Running Queue Stress..." << std::endl;

        unsigned int ii;
        for(ii = 0; ii < _aThreadData.size(); ++ii)
        {
            _aThreadData[ii].pStress = this;
            _aThreadData[ii].thread_num = ii;
        }

        std::vector<HANDLE> aHandles(NUMTHREADS);
        for(ii = 0; ii < aHandles.size(); ++ii)
        {
            unsigned int tid;
            aHandles[ii] = (HANDLE)_beginthreadex(nullptr, 0, QueueThreadFunc, &_aThreadData[ii], 0, &tid);
        }

        //
        // Wait for the threads to exit.
        //
        std::for_each(aHandles.begin(), aHandles.end(), HandleWait);

        //
        // Ideas for improvement:
        //  We could verify that there is a 1-1 mapping between values added and values removed.
        //  Verify the count of pops in the queue matches the number of pops for each thread.
        //
    } // void operator()()

    static unsigned int __stdcall QueueThreadFunc(_In_ void * pv)
    {
        unsigned int tid = GetCurrentThreadId();
        ThreadData * ptd = reinterpret_cast<ThreadData *>(pv);
        if(FULL_TRACE)
        {
            std::cout << tid << " adding" << std::endl;
        }

        unsigned int ii;
        for(ii = 0; ii < cNodes; ++ii)
        {
            ptd->pStress->_queue.Add(ptd->pStress->_apNodes[ptd->thread_num * cNodes + ii + 1]);
        }

        if(FULL_TRACE)
        {
            std::cout << tid << " removing" << std::endl;
        }

        for(ii = 0; ii < cNodes; ++ii)
        {
            ptd->pStress->_queue.Remove();
        }

        return 0;
    }
};  // class StressQueue

//
// Demonstrate the lock-free freelist.
// The freelist is based off of ideas found in the freelist article in Game
// Programming Gems 4 by Paul Glinker.  Other ideas for improvement can be
// found in the freelist article in Game Programming Gems 5 by Nathan Mefford.
//
void Demo_Freelist()
{
    std::cout << "Demo of Freelist...";

    //
    // Create a Freelist of MyStructs with 10 elements.
    //
    LockFreeFreeList<MyStruct> fl(10);

    //
    // Allocate a new MyStruct object.
    //
    MyStruct * pStruct = fl.NewInstance();

    //
    // Destroy the MyStruct object and return it to the Freelist.
    //
    fl.FreeInstance(pStruct);

    std::cout << "done" << std::endl;
}

//
// Test the lock-free implementations.
//
int main()
{
    //
    // Test CAS and CAS2
    //
    Test_CAS_assembly();
    Test_CAS_intrinsic();
    Test_CAS_windows();

    Test_CAS2_assembly();
    Test_CAS2_intrinsic();
    Test_CAS2_windows();

    //
    // Test Lock-free Stack
    //
    StressStack<TEST_TYPE, 4096>()();

    // Demo the stack
    node<MyStruct> Nodes[10];

    LockFreeStack<MyStruct> stack;
    stack.Push(&Nodes[1]);
    stack.Pop();        // returns &Nodes[1]
    stack.Pop();        // returns nullptr

    //
    // Test Lock-free Queue
    //
    std::vector<node<TEST_TYPE> *> apNodes(StressQueue<TEST_TYPE, 4096>::cNodes * 4096 + 1);    // 4096 threads, and 1 extra dummy node
    std::for_each(apNodes.begin(), apNodes.end(), CreateNode<TEST_TYPE>);

    StressQueue<TEST_TYPE, 4096> theQueue(apNodes);
    theQueue();

    std::for_each(apNodes.begin(), apNodes.end(), DeleteNode<TEST_TYPE>);

    // Demo the queue
    LockFreeQueue<MyStruct> queue(&Nodes[0]);   // Nodes[0] is dummy node

    queue.Add(&Nodes[1]);
    queue.Remove();     // returns &Nodes[1]
    queue.Remove();     // returns nullptr;

    //
    // Demonstrate Lock-free Freelist
    //
    Demo_Freelist();

    return 0;
}

