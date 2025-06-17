#include <string>
#include <iostream>

// Do not initialize m_Count to 1 on creation because of MySharedPtr<T> usage. 
// This will wrongfully set the counter to 1 when no actual instance is created.
class Counter
{
public:
    Counter() 
    {
        m_Count = 1; 
    } 
    ~Counter() {}

    void Increase()
    {
        m_Count++;
    }

    void Decrease()
    {
        m_Count--;
    }

    int GetCount()
    {
        return m_Count;
    }

private:
    int m_Count = 0;
};

template <typename T> 
class MySharedPtr
{
public:
    MySharedPtr() : m_Ptr(nullptr), m_Counter(nullptr) {} // Default initializers
    explicit MySharedPtr(T* ptr) : m_Ptr(ptr), m_Counter(new Counter()) {} // Explicitly initializer
    ~MySharedPtr()
    {
        if (m_Counter != nullptr)
        {
            m_Counter->Decrease();
            if (m_Counter->GetCount() == 0)
            {
                delete m_Counter;
                delete m_Ptr;
                m_Counter = nullptr; // Explicitly set to null after pointer deletion.
                m_Ptr = nullptr; // Explicitly set to null after pointer deletion
            }
        }
    }

    // This is a sp(otherSharedPtr) call, creating a new reference
    MySharedPtr(const MySharedPtr<T>& other)
    {
        // This is shallow copy and it is intended
        m_Ptr = other.m_Ptr;
        m_Counter = other.m_Counter;

        m_Counter->Increase();
    }

    // This is a copy constructor
    const MySharedPtr<T>& operator=(const MySharedPtr<T>& other)
    {
        if (m_Ptr != other.m_Ptr)
        {
            m_Ptr = other.m_Ptr;
            m_Counter = other.m_Counter;
            m_Counter->Increase();
        }

        return *this;
    }


    T* get()
    {
        return m_Ptr;
    }

    T* operator->() 
    {
        return m_Ptr;
    }

    T& operator*() 
    {
        return *m_Ptr;
    }

    int GetCount()
    {
        // When MySharedPtr<T> is called, accessing the counter is 0. It is technically not shared yet.
        return m_Counter != nullptr ? m_Counter->GetCount() : 0;
    }

private:
    T* m_Ptr;   
    Counter* m_Counter; // Must be pointer, otherwise count is not sharable. If make static, it can only trace 1 type.
};

class Test
{
    int value { 23 };
    std::string str { "sharedptr" };
};
 
int main()
{
    MySharedPtr<Test> sp0;
    std::cout << "sp0: " <<  sp0.GetCount() << std::endl;
    {
        MySharedPtr<Test> sp1(new Test());
        std::cout << "sp1: " << sp1.GetCount() << std::endl;

        MySharedPtr<Test> sp2(sp1);
        std::cout << "sp2: " << sp2.GetCount() << std::endl;

        sp0 = sp2;
        std::cout<< "sp0: " << sp0.GetCount() << std::endl;
    }
    std::cout << "sp0: " << sp0.GetCount() << std::endl;
    return 0;
}