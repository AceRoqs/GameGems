#ifndef LFFREELIST_H
#define LFFREELIST_H

#include "lfstack.h"

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
    LockFreeFreeList(uint32_t cObjects);
    ~LockFreeFreeList();
    void FreeAll();
    Ty * NewInstance();
    void FreeInstance(_In_bytecount_c_(sizeof node<Ty>) Ty * pInstance);
};

//
// cObjects is passed to the constructor instead of a template parameter
// to minimize the code bloat of multiple freelists with varying sizes,
// but each using the same underlying type.
//
template<typename Ty>
LockFreeFreeList<Ty>::LockFreeFreeList(uint32_t cObjects) : _cObjects(cObjects)
{
    //
    // The Freelist may live on the stack, so we allocate the
    // actual nodes on the heap to minimize the space hit.
    //
    _pObjects = new node<Ty>[cObjects];
    FreeAll();
}

template<typename Ty>
LockFreeFreeList<Ty>::~LockFreeFreeList()
{
#ifdef _DEBUG
    for(uint32_t ix = 0; ix < _cObjects; ++ix)
    {
        assert(_Freelist.Pop() != nullptr);
    }
#endif

    delete[] _pObjects;
}

template<typename Ty>
void LockFreeFreeList<Ty>::FreeAll()
{
    for(uint32_t ix = 0; ix < _cObjects; ++ix)
    {
        _Freelist.Push(&_pObjects[ix]);
    }
}

template<typename Ty>
Ty * LockFreeFreeList<Ty>::NewInstance()
{
    node<Ty> * pInstance = _Freelist.Pop();
    return new(&pInstance->value) Ty;
}

// This is the best annotation possible given that the code hides
// that a node structure is actually what is being passed.
// This will not prevent an unwrapped 'Ty' from being passed.
template<typename Ty>
void LockFreeFreeList<Ty>::FreeInstance(_In_bytecount_c_(sizeof node<Ty>) Ty * pInstance)
{
    pInstance->~Ty();
    _Freelist.Push(reinterpret_cast<node<Ty> *>(pInstance));
}

#endif

