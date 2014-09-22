#ifndef LFQUEUE_H
#define LFQUEUE_H

#include "lfcas.h"

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
    LockFreeQueue(_In_ node<Ty> * pDummy);

    void Add(_In_ node<Ty> * pNode);
    node<Ty> * Remove();
};

template<typename Ty>
LockFreeQueue<Ty>::LockFreeQueue(_In_ node<Ty> * pDummy) : _cPops(0), _cPushes(0)
{
    _pHead = _pTail = pDummy;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#endif

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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

