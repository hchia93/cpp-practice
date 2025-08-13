#include <utility>
#include <iostream>

template <typename T>
class ListNode
{
public:
    ListNode* pNext = nullptr;
    T* pValue = nullptr;

    ListNode() : pValue(new T()), pNext(nullptr) {};
    ListNode(const T& value) : pValue(value), pNext(nullptr) {};
    ListNode(T&& value) : pValue(new T(std::move(value))), pNext(nullptr) {};
    ListNode(const T& value, ListNode* next) : pValue(value), pNext(next) {};
    ~ListNode()
    {
        delete pValue;
        pValue = nullptr;
        pNext = nullptr;
    }

    void Print() const
    {
        if (pValue)
            std::cout << *pValue;
        else
            std::cout << "null";
    }
};

namespace ListNodeHelper
{
    template <typename T>
    void PrintList(ListNode<T>* list)
    {
        const ListNode<T>* pCurrent = list;
        while (pCurrent != nullptr)
        {
            pCurrent->Print();
            std::cout << " -> ";
            pCurrent = pCurrent->pNext;
        }
        std::cout << "null" << std::endl;
    }

    template <typename T>
    bool HasLoop(ListNode<T>* list)
    {
        if (list == nullptr || list->pNext == nullptr)
        {
            return false;
        }
        
        ListNode<int>* slow = list;
        ListNode<int>* fast = list->pNext;

        while (fast != nullptr && fast->pNext != nullptr)
        {
            if (slow == fast)
            {
                return true;
            }
            slow = slow->pNext;
            fast = fast->pNext->pNext;
        }
        return false;
    }
}

ListNode<int>* CreateLoopedList()
{
    ListNode<int>* root = new ListNode<int>(1);
    ListNode<int>* tail = root;

    for (int i = 0; i < 5; i++)
    {
        tail->pNext = new ListNode<int>(i + 2);
        tail = tail->pNext;
    }
    tail->pNext = root->pNext;
    return root;
}

int main()
{
    ListNode<int>* loopingList = CreateLoopedList();
    if (ListNodeHelper::HasLoop(loopingList))
    {
        std::cout << "list has loop" << std::endl;
    }
    
    system("pause");
    return 0;
}