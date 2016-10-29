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
    node<Ty> * volatile _pHead = nullptr;
    volatile uint32_t  _cPops = 0;

public:
    void Push(_In_ node<Ty> * pNode);
    node<Ty> * Pop();
};

template<typename Ty>
void LockFreeStack<Ty>::Push(_In_bytecount_c_(sizeof node<Ty>) node<Ty> * pNode)
{
    for(;;)
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
    for(;;)
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

#endif

