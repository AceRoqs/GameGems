#ifndef LFSTACK_H
#define LFSTACK_H

#include "lfcas.h"

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
    LockFreeStack();

    void Push(_In_ node<Ty> * pNode);
    node<Ty> * Pop();
};

template<typename Ty>
LockFreeStack<Ty>::LockFreeStack() : _pHead(nullptr), _cPops(0)
{
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#endif

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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

