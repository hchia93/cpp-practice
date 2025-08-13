#include <utility>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <optional>
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
    void Print(ListNode<T>* list)
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
    bool ContainsLoop(ListNode<T>* list)
    {
        if (list == nullptr || list->pNext == nullptr)
        {
            return false;
        }
        
        ListNode<int>* slow = list;
        ListNode<int>* fast = list;

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
    
    template <typename T>
    std::optional<int> GetLoopEntryIndex(ListNode<T>* head)
    {
        if (!head || !head->pNext)
        {
            return std::nullopt;
        }

        ListNode<T>* slow = head;
        ListNode<T>* fast = head;

        while (fast && fast->pNext)
        {
            slow = slow->pNext;
            fast = fast->pNext->pNext;

            if (slow == fast)
            {
                break;
            }
        }

        if (!fast || !fast->pNext)
        {
            return std::nullopt;
        }


        ListNode<T>* entry = head;
        while (entry != slow)
        {
            entry = entry->pNext;
            slow = slow->pNext;
        }

        int index = 0;
        for (ListNode<T>* cur = head; cur != entry; cur = cur->pNext)
        {
            ++index;
        }
        return index;
    }
}

namespace LinkedListGenerator
{
    ListNode<int>* CreateList_NumericLoopedList(int maxNumber)
    {
        ListNode<int>* pRoot = new ListNode<int>(1);
        ListNode<int>* pCurrent = pRoot;
        ListNode<int>* pLoop = pRoot;

        int selection = rand() % (maxNumber + 1);
        std::cout << "Loop index picked at : " << std::clamp(selection - 1, 0, maxNumber) << std::endl;

        for (int i = 0; i < maxNumber - 1; ++i)
        {
            pCurrent->pNext = new ListNode<int>(i + 2);
            pCurrent = pCurrent->pNext;

            if (i + 2 == selection)
            {
                pLoop = pCurrent;
            }
        }
        pCurrent->pNext = pLoop;
        return pRoot;
    };
}

int main()
{
    srand(time(NULL));
    ListNode<int>* list = LinkedListGenerator::CreateList_NumericLoopedList(5);
    if (ListNodeHelper::ContainsLoop(list))
    {
        std::cout << "list has loop" << std::endl;
    }
    else
    {
        ListNodeHelper::Print(list);
    }

    std::optional result = ListNodeHelper::GetLoopEntryIndex(list);
    if (result.has_value())
    {
        std::cout << "loop entry index detected at : " << result.value() << std::endl;
    }
    
    system("pause");
    return 0;
}