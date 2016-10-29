#ifndef CAS_H
#define CAS_H

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
    node<Ty> * volatile pNext = nullptr;

    node() : value() {}
    node(Ty v) : value(v) {}
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
template<typename Ty>
bool CAS_intrinsic(_Inout_ node<Ty> * volatile * _ptr, node<Ty> * oldVal, node<Ty> * newVal)
{
#ifdef _X86_
    return _InterlockedCompareExchange(reinterpret_cast<long volatile *>(_ptr),
                                       reinterpret_cast<intptr_t>(newVal),
                                       reinterpret_cast<intptr_t>(oldVal)) == reinterpret_cast<intptr_t>(oldVal);
#else
    return _InterlockedCompareExchange64(reinterpret_cast<__int64 volatile *>(_ptr),
                                         reinterpret_cast<intptr_t>(newVal),
                                         reinterpret_cast<intptr_t>(oldVal)) == reinterpret_cast<intptr_t>(oldVal);
#endif
}
#endif  // _MSC_VER

//
// Define a version of CAS which uses the Windows API InterlockedCompareExchange.
//
template<typename Ty>
bool CAS_windows(_Inout_ node<Ty> * volatile * _ptr, node<Ty> * oldVal, node<Ty> * newVal)
{
#ifdef _X86_
    return InterlockedCompareExchange(reinterpret_cast<long volatile *>(_ptr),
                                      reinterpret_cast<intptr_t>(newVal),
                                      reinterpret_cast<intptr_t>(oldVal)) == reinterpret_cast<intptr_t>(oldVal);
#else
    return InterlockedCompareExchange64(reinterpret_cast<__int64 volatile *>(_ptr),
                                        reinterpret_cast<intptr_t>(newVal),
                                        reinterpret_cast<intptr_t>(oldVal)) == reinterpret_cast<intptr_t>(oldVal);
#endif
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
template<typename Ty>
bool CAS2_intrinsic(_Inout_count_c_(2) node<Ty> * volatile * _ptr, node<Ty> * old1, uint32_t old2, node<Ty> * new1, uint32_t new2)
{
    // For x64 architectures, _InterlockedCompareExchange128 can be used, but cmpxchg16b is not
    // supported on all AMD CPUs.
    // http://msdn.microsoft.com/en-us/library/bb514094.aspx
    static_assert(sizeof(old1) == sizeof(long), "CAS2_intrinsic not supported on this architecture.");

    LONGLONG Comperand = reinterpret_cast<long>(old1) | (static_cast<LONGLONG>(old2) << 32);
    LONGLONG Exchange  = reinterpret_cast<long>(new1) | (static_cast<LONGLONG>(new2) << 32);

    return _InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile *>(_ptr), Exchange, Comperand) == Comperand;
}
#endif  // _MSC_VER

//
// Define a version of CAS2 which uses the Windows API InterlockedCompareExchange64.
// InterlockedCompareExchange64 requires Windows Vista.
//
#if WINVER >= 0x0600

template<typename Ty>
bool CAS2_windows(_Inout_count_c_(2) node<Ty> * volatile * _ptr, node<Ty> * old1, uint32_t old2, node<Ty> * new1, uint32_t new2)
{
    static_assert(sizeof(old1) == sizeof(long), "CAS2_windows not supported on this architecture.");

    LONGLONG Comperand = reinterpret_cast<long>(old1) | (static_cast<LONGLONG>(old2) << 32);
    LONGLONG Exchange  = reinterpret_cast<long>(new1) | (static_cast<LONGLONG>(new2) << 32);

    return InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile *>(_ptr), Exchange, Comperand) == Comperand;
}
#endif  // WINVER >= 0x0600

#endif

