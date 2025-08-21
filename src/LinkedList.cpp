#include <utility>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <optional>
#include <vector>
#include <iostream>
#include <initializer_list>
#include <functional>
#include <variant>

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

	struct LoopDetector
	{
		// LeetCode 141 - Detect if a loop exists in the linked list
		template <typename T>
		static ListNode<T>* DetectMeetingPoint(ListNode<T>* head)
		{
			ListNode<T>* slow = head;
			ListNode<T>* fast = head;
			while (fast && fast->pNext)
			{
				slow = slow->pNext;
				fast = fast->pNext->pNext;
				if (slow == fast)
				{
					return slow;
				}
			}
			return nullptr;
		}

		// LeetCode 142 - Find the entry point of the loop
		template <typename T>
		static ListNode<T>* FindLoopEntry(ListNode<T>* head, ListNode<T>* meeting)
		{
			if (!meeting)
			{
				return nullptr;
			}

			ListNode<T>* slow = head;
			ListNode<T>* fast = meeting;
			while (slow != fast)
			{
				slow = slow->pNext;
				fast = fast->pNext;
			}
			return slow;
		}

		template <typename T>
		static ListNode<T>* FindLoopTail(ListNode<T>* entry)
		{
			if (!entry)
			{
				return nullptr;
			}

			ListNode<T>* current = entry;
			while (current->pNext != entry)
			{
				current = current->pNext;
			}
			return current;
		}
	};

	template <typename T>
	void BreakLoop(ListNode<T>* head)
	{
		auto meeting = LoopDetector::DetectMeetingPoint(head);
		if (!meeting)
		{
			return;
		}

		auto entry = LoopDetector::FindLoopEntry(head, meeting);
		auto tail = LoopDetector::FindLoopTail(entry);
		if (tail)
		{
			tail->pNext = nullptr;
		}
	}

	template <typename T>
	bool ContainsLoop(ListNode<T>* head)
	{
		return LoopDetector::DetectMeetingPoint(head) != nullptr;
	}

	template <typename T>
	std::optional<int> GetLoopEntryIndex(ListNode<T>* head)
	{
		auto meeting = LoopDetector::DetectMeetingPoint(head);
		if (!meeting) return std::nullopt;

		auto entry = LoopDetector::FindLoopEntry(head, meeting);

		int index = 0;
		for (auto cur = head; cur != entry; cur = cur->pNext)
		{
			++index;
		}
		return index;
	}

	struct IntersectionDetector
	{
		// LeetCode 160 - Detect intersection of two linked lists
		template <typename T>
		static ListNode<T>* DetectIntersection(ListNode<T>* head1, ListNode<T>* head2)
		{
			if (!head1 || !head2)
			{
				return nullptr;
			}

			ListNode<T>* current1 = head1;
			ListNode<T>* current2 = head2;

			while (current1 != current2)
			{
				current1 = (current1 == nullptr) ? head2 : current1->pNext;
				current2 = (current2 == nullptr) ? head1 : current2->pNext;
			}
			return current1;
		}
	};;

	template <typename T>
	void FreeList(ListNode<T>* head)
	{
		if (!head)
		{
			return;
		}

		BreakLoop(head);

		while (head != nullptr)
		{
			ListNode<T>* next = head->pNext;
			delete head;
			head = next;
		}
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

	// Leet Code 203 - Remove all nodes with a specific value
	template <typename T>
	ListNode<T>* RemoveElement(ListNode<T>* head, const T& value)
	{
		ListNode<T>* dummy = new ListNode<T>();
		dummy->pNext = head;
	
		ListNode<T>* previous = dummy;
		ListNode<T>* current = head;

		while (current != nullptr)
		{
			if (*(current->pValue) == value)
			{
				// Correct the link to skip the current node
				previous->pNext = current->pNext;
				delete current;
				current = previous->pNext;
			}
			else
			{
				// Move both pointers
				previous = current;
				current = current->pNext;
			}
		}

		ListNode<T>* newHead = dummy->pNext;
		delete dummy;
		return newHead;
	}

	template <typename T>
	ListNode<T>* Merge(ListNode<T>* head1, ListNode<T>* head2)
	{
		ListNode<T> dummy;
		ListNode<T>* tail = &dummy;

		while (head1 != nullptr && head2 != nullptr)
		{
			if (*(head1->pValue) < *(head2->pValue))
			{
				tail->pNext = head1;
				head1 = head1->pNext;
			}
			else
			{
				tail->pNext = head2;
				head2 = head2->pNext;
			}
			tail = tail->pNext;
		}
		tail->pNext = head1 ? head1 : head2;
		return dummy.pNext;
	}

	// Leet Code 148 - Sort a linked list into ascending order
	template <typename T>
	ListNode<T>* Sort(ListNode<T>* head)
	{
		if (!head || !head->pNext)
		{
			return head;
		}

		ListNode<T>* slow = head;
		ListNode<T>* fast = head;
		ListNode<T>* previous = nullptr;

		while (fast && fast->pNext)
		{
			previous = slow;
			slow = slow->pNext;
			fast = fast->pNext->pNext;
		}

		if (previous)
		{
			previous->pNext = nullptr;
		}

		ListNode<T>* left = Sort(head);
		ListNode<T>* right = Sort(slow);

		return Merge(left, right);
	}

	// Leet Code 206 - Reverse a linked list
	template <typename T>
	ListNode<T>* Reverse(ListNode<T>* head)
	{
		ListNode<T> dummy;
		while (head)
		{
			ListNode<T>* next = head->pNext;
			head->pNext = dummy.pNext;
			dummy.pNext = head;
			head = next;

		}
		return dummy.pNext;
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
	// Extends if test function returns more possible types.
	template <typename T>
	using TestResult = std::variant<ListNode<T>*, bool, std::optional<int>>;

	template <typename T>
	static void MakeTest(const char* testName, const std::function<TestResult<T>()>& testFunction)
	{
		TestResult<T> result = testFunction();
		std::cout << testName << " (Solution) : ";

		std::visit([&](auto&& value)
			{
				using U = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<U, ListNode<T>*>)
				{
					ListNodeHelper::Print(value);
					ListNodeHelper::FreeList(value);
				}
				else if constexpr (std::is_same_v<U, bool>)
				{
					std::cout << (value ? "true" : "false") << std::endl;

					// WARNING : FREED THE LIST IN THE LAMBDA
				}
				else if constexpr (std::is_same_v<U, std::optional<int>>)
				{
					if (value)
					{
						std::cout << *value << std::endl;
					}
					else
					{
						std::cout << "nullopt" << std::endl;
					}

					// WARNING : FREED THE LIST IN THE LAMBDA
				}

			}, result);
	}
}

int main()
{
	// Test cases for LeetCode 82 (Remove all duplicates)
	ListNodeTester::MakeTest<int>("No82a", [] { 
		auto list = ListNodeCreator::MakeNumericList({ 1, 2, 3, 3, 3, 4, 4, 5 }); 
		ListNodeHelper::Print("No82a", list);  
		return ListNodeTester::TestResult<int>(ListNodeHelper::RemoveDuplicates(list)); 
		});

	ListNodeTester::MakeTest<int>("No82b", [] {
		auto list = ListNodeCreator::MakeNumericList({ 1, 1, 1, 2, 3 }); 
		ListNodeHelper::Print("No82b", list); 
		return ListNodeTester::TestResult<int>(ListNodeHelper::RemoveDuplicates(list)); 
		});

	ListNodeTester::MakeTest<int>("No82c", [] { 
		auto list = ListNodeCreator::MakeNumericList({ 1, 1, 2, 2 }); 
		ListNodeHelper::Print("No82c", list); 
		return ListNodeTester::TestResult<int>(ListNodeHelper::RemoveDuplicates(list)); 
		});

	ListNodeTester::MakeTest<int>("No82d", [] { 
		auto list = ListNodeCreator::MakeNumericList({ 1, 1 }); 
		ListNodeHelper::Print("No82d", list); 
		return ListNodeTester::TestResult<int>(ListNodeHelper::RemoveDuplicates(list)); 
		});

	ListNodeTester::MakeTest<int>("No82e", [] { 
		auto list = ListNodeCreator::MakeNumericList({ 1, 2, 2 }); 
		ListNodeHelper::Print("No82e", list); 
		return ListNodeTester::TestResult<int>(ListNodeHelper::RemoveDuplicates(list)); 
		});

	ListNodeTester::MakeTest<int>("No82f", [] {
		auto list = ListNodeCreator::MakeNumericList({ 1, 1, 2, 2, 3, 3 });
		ListNodeHelper::Print("No82f", list); 
		return ListNodeTester::TestResult<int>(ListNodeHelper::RemoveDuplicates(list)); 
		});

	ListNodeTester::MakeTest<int>("No83", [] { 
		auto list = ListNodeCreator::MakeNumericList({ 1, 1, 1, 2, 2, 3 }); 
		ListNodeHelper::Print("No83", list);
		return ListNodeTester::TestResult<int>(ListNodeHelper::MakeUnique(list)); 
		});

	ListNodeTester::MakeTest<int>("No141", [] { 
		auto list = ListNodeCreator::MakeNumericListLooped(8); 
		bool result = ListNodeHelper::ContainsLoop(list); 
		// DO NOT PRINT THIS LIST, IT IS LOOPED
		ListNodeHelper::FreeList(list); 
		return ListNodeTester::TestResult<int>(result);	
		});

	ListNodeTester::MakeTest<int>("No142", [] { 
		auto list = ListNodeCreator::MakeNumericListLooped(16); 
		std::optional<int> result = ListNodeHelper::GetLoopEntryIndex(list); 
		// DO NOT PRINT THIS LIST, IT IS LOOPED
		ListNodeHelper::FreeList(list); 
		return ListNodeTester::TestResult<int>(result); 
		});

	ListNodeTester::MakeTest<int>("Merge1", [] {
		auto list1 = ListNodeCreator::MakeNumericList({ 1, 3, 5 });
		auto list2 = ListNodeCreator::MakeNumericList({ 2, 4, 6 });
		ListNodeHelper::Print("Merge1", list1);
		ListNodeHelper::Print("Merge1", list2);
		return ListNodeTester::TestResult<int>(ListNodeHelper::Merge(list1, list2));
		});

	ListNodeTester::MakeTest<int>("Merge2", [] {
		auto list1 = ListNodeCreator::MakeNumericList({ 1, 2, 3 });
		auto list2 = (ListNode<int>*)nullptr; // empty
		ListNodeHelper::Print("Merge2", list1);
		ListNodeHelper::Print("Merge2", list2);
		return ListNodeTester::TestResult<int>(ListNodeHelper::Merge(list1, list2));
		});

	ListNodeTester::MakeTest<int>("Merge3", [] {
		auto list1 = ListNodeCreator::MakeNumericList({ 1, 5, 7 });
		auto list2 = ListNodeCreator::MakeNumericList({ 4 });
		ListNodeHelper::Print("Merge3", list1);
		ListNodeHelper::Print("Merge3", list2);
		return ListNodeTester::TestResult<int>(ListNodeHelper::Merge(list1, list2));
		});

	ListNodeTester::MakeTest<int>("Merge4", [] {
		auto list1 = (ListNode<int>*)nullptr;
		auto list2 = (ListNode<int>*)nullptr;
		ListNodeHelper::Print("Merge4", list1);
		ListNodeHelper::Print("Merge4", list2);
		return ListNodeTester::TestResult<int>(ListNodeHelper::Merge(list1, list2));
		});

	ListNodeTester::MakeTest<int>("Merge5", [] {
		auto list1 = ListNodeCreator::MakeNumericList({ 1, 2, 2, 4 });
		auto list2 = ListNodeCreator::MakeNumericList({ 2, 3, 4 });
		ListNodeHelper::Print("Merge5", list1);
		ListNodeHelper::Print("Merge5", list2);
		return ListNodeTester::TestResult<int>(ListNodeHelper::Merge(list1, list2));
		});

	ListNodeTester::MakeTest<int>("No203", [] {
		auto list = ListNodeCreator::MakeNumericList({ 1, 2, 6, 3, 4, 5, 6 });
		return ListNodeTester::TestResult<int>(
			ListNodeHelper::RemoveElement(list, 2)
		);
		});

	ListNodeTester::MakeTest<int>("No206", [] {
		auto list = ListNodeCreator::MakeNumericList({ 1, 2, 3, 7, 8, 9 });
		ListNodeHelper::Print("No206", list);
		return ListNodeTester::TestResult<int>(
			ListNodeHelper::Reverse(list)
		);
		});

	system("pause");
	return 0;
}