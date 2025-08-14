#include <utility>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <optional>
#include <vector>
#include <iostream>
#include <initializer_list>
#include <functional>

template <typename T>
class ListNode
{
public:
    ListNode* pNext = nullptr;
    T* pValue = nullptr;

    ListNode() : pValue(new T()), pNext(nullptr) {};
    ListNode(const T& value) : pValue(new T(value)), pNext(nullptr) {};
    ListNode(T&& value) : pValue(new T(std::move(value))), pNext(nullptr) {};
    ListNode(const T& value, ListNode* next) : pValue(new T(value)), pNext(next) {};
    ~ListNode()
    {
        delete pValue;
        pValue = nullptr;
        pNext = nullptr;
    }

    void Print() const
    {
        if (pValue)
        {
            // Use a helper function to print the value safely
            PrintValue(*pValue);
        }
        else
        {
            std::cout << "null";
        }  
    }

private:
    // Helper function to print values - can be specialized for different types
    template<typename U = T>
    void PrintValue(const U& value) const
    {
        std::cout << value;
    }
    
    // Specialization for void* or other non-printable types
    template<typename U = T>
    void PrintValue(const U*& value) const
    {
        std::cout << "ptr:" << static_cast<const void*>(value);
    }
};

namespace ListNodeHelper
{
    template <typename T>
    void Print(ListNode<T>* head)
    {
        const ListNode<T>* curent = head;
        while (curent != nullptr)
        {
            curent->Print();
            std::cout << " -> ";
            curent = curent->pNext;
        }
        std::cout << "null" << std::endl;
    }

    template <typename T>
    void Print(const std::string& label, ListNode<T>* head)
    {
        std::cout << label << ": ";
        Print(head);
    }

    template <typename T>
    bool ContainsLoop(ListNode<T>* head)
    {
        if (head == nullptr || head->pNext == nullptr)
        {
            return false;
        }
        
        ListNode<int>* slow = head;
        ListNode<int>* fast = head;

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
        if (head == nullptr || head->pNext == nullptr)
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
        for (ListNode<T>* current = head; current != entry; current = current->pNext)
        {
            ++index;
        }
        return index;
    }

    // Leet Code 82 - If a node is ever duplicated, remove them entirely
    template <typename T>
    ListNode<T>* RemoveDuplicates(ListNode<T>* head)
    {
        if (head != nullptr)
        {
            ListNode<T>* previous = nullptr;
            ListNode<T>* current = head;
            ListNode<T>* node = nullptr;

            while (current->pNext != nullptr)
            {
                if (current->pValue != nullptr && current->pNext->pValue != nullptr && *current->pValue == *current->pNext->pValue)
                {
                    // additionally mark current node to delete after deleting all trailing duplicates. 
                    node = current;

                    // deleting trailing duplicates
                    ListNode<T>* next = current->pNext;
                    current->pNext = (next != nullptr) ? next->pNext : nullptr;
                    delete next;
                    next = nullptr;
                }
                else
                {
                    if (node != nullptr)
                    {
                        if (previous != nullptr)
                        {
                            previous->pNext = current->pNext;
                            delete node;
                            node = nullptr;

                            current = previous->pNext;
                        }
                        else // previous is nullptr, promoting new head.
                        {
                            head = current->pNext;
                            delete node;
                            node = nullptr;

                            // old head removed, assign to new head.
                            current = head;
                        }
                    }
                    else
                    {
                        previous = current;
                        current = current->pNext;
                    }
                }
            }
            
            // if current node is still marked delete when there is no next node (unable to handle in loop) 
            if (node != nullptr)
            {
                // promoting head to last known previous node
                if (previous != nullptr)
                {
                    previous->pNext = nullptr;
                    delete node;
                    node = nullptr;
                }
                else // promoting head to nullptr when there is no more previous node
                {
                    return nullptr;
                }
            }
        }

        return head;
    }

    // Leet Code 83 - Remove all duplicate nodes
    template <typename T>
    ListNode<T>* MakeUnique(ListNode<T>* head)
    {
        if (head != nullptr)
        {
            ListNode<T>* current = head;

            while (current->pNext != nullptr)
            {
                if (current->pValue && current->pNext->pValue && *current->pValue == *current->pNext->pValue)
                {
                    ListNode<T>* node = current->pNext;
                    current->pNext = node->pNext;
                    delete node;
                    node = nullptr;
                }
                else
                {
                    current = current->pNext;
                }
            }
        }

        return head;
    }
}

namespace ListNodeCreator
{
    ListNode<int>* MakeNumericListLooped(int maxNumber)
    {
        ListNode<int>* head = new ListNode<int>(1);
        ListNode<int>* current = head;
        ListNode<int>* loop = head;

        srand(time(NULL));
        int selection = rand() % (maxNumber + 1);
        std::cout << "Loop index picked at : " << (selection - 1 < 0 ? 0 : (selection - 1 > maxNumber ? maxNumber : selection - 1)) << std::endl;

        for (int i = 0; i < maxNumber - 1; ++i)
        {
            current->pNext = new ListNode<int>(i + 2);
            current = current->pNext;

            if (i + 2 == selection)
            {
                loop = current;
            }
        }
        current->pNext = loop;
        return head;
    }

    ListNode<int>* MakeNumericList(std::vector<int>& intList)
    {
        if (intList.empty())
        {
            return nullptr;
        }

        ListNode<int>* head = new ListNode<int>(intList[0]);
        ListNode<int>* current = head;
        for (int i = 1; i < intList.size(); ++i)
        {
            ListNode<int>* pNode = new ListNode<int>(intList[i]);
            current->pNext = pNode;
            current = current->pNext;
        }
        return head;
    }

    ListNode<int>* MakeNumericList(std::initializer_list<int> intList)
    {
        if (intList.size() == 0)
        {
            return nullptr;
        }

        auto it = intList.begin();
        ListNode<int>* head = new ListNode<int>(*it);
        ListNode<int>* current = head;
        ++it;

        for (; it != intList.end(); ++it)
        {
            ListNode<int>* pNode = new ListNode<int>(*it);
            current->pNext = pNode;
            current = current->pNext;
        }
        return head;
    }
}

namespace ListNodeTester
{
    static void MakeTest(const char* testName, std::vector<int>& data, const std::function<ListNode<int>* (ListNode<int>*)>& testFunction)
    {
        ListNode<int>* list = ListNodeCreator::MakeNumericList(data);
        ListNodeHelper::Print(testName, list);

        if (testFunction != nullptr)
        {
            ListNode<int>* result = testFunction(list);
            ListNodeHelper::Print("Solution", result);

            if (result != nullptr)
            {
                delete result;
            }
        }
        else
        {
            delete list;
        }

        std::cout << std::endl;
    }

    static void MakeTest(const char* testName, std::initializer_list<int> intList, const std::function<ListNode<int>* (ListNode<int>*)>& testFunction)
    {
        ListNode<int>* list = ListNodeCreator::MakeNumericList(intList);
        ListNodeHelper::Print(testName, list);

        if (testFunction != nullptr)
        {
            ListNode<int>* result = testFunction(list);
            ListNodeHelper::Print("Solution", result);

            if (result != nullptr)
            {
                delete result;
            }
        }
        else
        {
            delete list;
        }

        std::cout << std::endl << std::endl;
    }
}

int main()
{
    // Test cases for LeetCode 82 (Remove all duplicates)
    ListNodeTester::MakeTest("No82a", { 1, 2, 3, 3, 3, 4, 4, 5 }, &ListNodeHelper::RemoveDuplicates<int>);
    ListNodeTester::MakeTest("No82b", { 1, 1, 1, 2, 3 }, &ListNodeHelper::RemoveDuplicates<int>);
    ListNodeTester::MakeTest("No82c", { 1, 1, 2, 2 }, &ListNodeHelper::RemoveDuplicates<int>);
    ListNodeTester::MakeTest("No82d", { 1, 1 }, &ListNodeHelper::RemoveDuplicates<int>);
    ListNodeTester::MakeTest("No82e", { 1, 2, 2 }, &ListNodeHelper::RemoveDuplicates<int>);
    ListNodeTester::MakeTest("No82f", { 1, 1, 2, 2, 3, 3 }, &ListNodeHelper::RemoveDuplicates<int>);

    // Test case for LeetCode 83 (Make Unique)
    ListNodeTester::MakeTest("No83", { 1, 1, 1, 2, 2, 3 }, &ListNodeHelper::MakeUnique<int>);

    // Loop Detection
    ListNode<int>* list1 = ListNodeCreator::MakeNumericListLooped(5);
    if (ListNodeHelper::ContainsLoop(list1))
    {
        std::cout << "list has loop" << std::endl;
    }
    else
    {
        ListNodeHelper::Print(list1);
    }

    std::optional<int> result = ListNodeHelper::GetLoopEntryIndex(list1);
    if (result.has_value())
    {
        std::cout << "loop entry index detected at : " << result.value() << std::endl;
    }
    
    // Initializer list test
    ListNode<int>* list2 = ListNodeCreator::MakeNumericList({ 1, 2, 3, 4, 5 });
    std::cout << "Initializer list test: ";
    ListNodeHelper::Print(list2);

    system("pause");
    return 0;
}