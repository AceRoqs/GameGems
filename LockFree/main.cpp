//
// Game Programming Gems 6
// Lock free multithreaded algorithms
// By Toby Jones
// Supports Microsoft Visual C++ and GCC.
//

#include "PreCompile.h"
#include "lfqueue.h"
#include "lffreelist.h"

#ifdef _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant
#endif

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
        std::for_each(std::begin(_apNodes), std::end(_apNodes), CreateNode<Ty>);

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
            aHandles[ii] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, StackThreadFunc, &_aThreadData[ii], 0, &tid));
        }

        //
        // Wait for the threads to exit.
        //
        std::for_each(std::begin(aHandles), std::end(aHandles), HandleWait);

        //
        // Delete all of the nodes.
        //
        std::for_each(std::begin(_apNodes), std::end(_apNodes), DeleteNode<Ty>);

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
            aHandles[ii] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, QueueThreadFunc, &_aThreadData[ii], 0, &tid));
        }

        //
        // Wait for the threads to exit.
        //
        std::for_each(std::begin(aHandles), std::end(aHandles), HandleWait);

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
    std::for_each(std::begin(apNodes), std::end(apNodes), CreateNode<TEST_TYPE>);

    StressQueue<TEST_TYPE, 4096> theQueue(apNodes);
    theQueue();

    std::for_each(std::begin(apNodes), std::end(apNodes), DeleteNode<TEST_TYPE>);

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

