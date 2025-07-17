#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <stdexcept>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <optional>
#include <charconv>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

enum class EUserAction
{
    BUY,
    SELL,
    CANCEL,
    MODIFY,
    PRINT,
    EXIT
};

enum class EOrderType
{
    IOC,
    GFD
};

enum class ETransactType
{
    BUY,
    SELL  
};

class Transaction
{
public:
    Transaction(); 
    Transaction(ETransactType transactionType, EOrderType orderType, int price, int quantity, std::string orderID)
    : m_TransactionType(transactionType), m_OrderType(orderType), m_Price(price), m_Quantity(quantity), m_OrderID(orderID)
    {
        
    }
    
    Transaction(const Transaction& other) 
    : m_TransactionType(other.m_TransactionType), m_OrderType(other.m_OrderType), m_Price(other.m_Price), m_Quantity(other.m_Quantity), m_OrderID(other.m_OrderID)
    {
        
    }
    
    std::string GetOrderID() const { return m_OrderID; }
    ETransactType GetTransactionType() const { return m_TransactionType; }
    EOrderType GetOrderType() const { return m_OrderType; }
    int GetPrice() const { return m_Price; }
    int GetQuantity() const { return m_Quantity; }
    
    bool CanBeModified() const
    {
        return m_OrderType != EOrderType::IOC;
    }
    
    void Modify(ETransactType transactionType, int price, int quantity)
    {
        m_TransactionType = transactionType;
        m_Price = price;
        m_Quantity = quantity;
    }
    
    void UpdateQuantity(int tradedQuantity)
    {
        m_Quantity -= tradedQuantity;
    }
    
private:
    ETransactType m_TransactionType;
    EOrderType m_OrderType;
    int m_Price;
    int m_Quantity;
    std::string m_OrderID;
};

class TransactionMarket
{
public:
    TransactionMarket() : m_StopThread(false)
    {
        m_MatchTradeThread = std::thread(&TransactionMarket::MatchThread, this);    
    }
    
    ~TransactionMarket()
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_StopThread = true;
        }
        m_CV.notify_all();
        
        if (m_MatchTradeThread.joinable())
        {
            m_MatchTradeThread.join();
        }
        
        for (Transaction* t : m_Transactions)
        {
            delete t;
        }
    }
    
    void CreateTransaction(ETransactType transactType, EOrderType orderType, int price, int quantity, std::string orderID)
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            Transaction* t = new Transaction(transactType, orderType, price, quantity, orderID); 
            m_Transactions.push_back(t);
        }
        m_CV.notify_one();
    }
    
    void CancelTransaction(std::string orderID)
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::vector<Transaction*>::iterator it = std::find_if(m_Transactions.begin(), m_Transactions.end(), [&orderID](Transaction* transaction)
            {
                return transaction->GetOrderID() == orderID;
            });
            
            if (it != m_Transactions.end())
            {
                delete *it;
                m_Transactions.erase(it);
            }
        }
        m_CV.notify_one();
        
    }
    
    void ModifyTransaction(std::string orderID, ETransactType transactType, int newPrice, int newQuantity)
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::vector<Transaction*>::iterator it = std::find_if(m_Transactions.begin(), m_Transactions.end(), [&orderID](Transaction* transaction)
            {
                return transaction->GetOrderID() == orderID;
            });
            
            if (it != m_Transactions.end())
            {
                Transaction* t = *it;
                if (t != nullptr && t->CanBeModified())
                {
                    Transaction* modifiedT = new Transaction(*t);
                    modifiedT->Modify(transactType, newPrice, newQuantity);
                    
                    // Keep the delete and recreate pattern to reset priority
                    m_Transactions.erase(it);
                    delete t;
                    
                    m_Transactions.push_back(modifiedT);
                }
            }
        }
        m_CV.notify_one();
    }
    
    void MatchThread()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_CV.wait(lock, [this] { return m_StopThread || !m_Transactions.empty(); });
            
            if (m_StopThread)
            {
                break;
            }
            
            MatchTransaction();
        }
    }
    
    void MatchTransaction()
    {
        // Set matching in progress flag to coordinate with other operations
        m_MatchingInProgress = true;
        
        std::vector<Transaction*> buys, sells;
        for (Transaction* t : m_Transactions)
        {
            if (t->GetTransactionType() == ETransactType::BUY)
            {
                buys.push_back(t);
            }
            else if (t->GetTransactionType() == ETransactType::SELL)
            {
                sells.push_back(t);
            }
        }
        
        // FIX: Sort sells in ascending order (lowest price first) for proper matching
        std::sort(buys.begin(), buys.end(), [](Transaction* a, Transaction* b) { return a->GetPrice() > b->GetPrice();});
        std::sort(sells.begin(), sells.end(), [](Transaction* a, Transaction*b) { return a->GetPrice() < b->GetPrice(); });
        
        std::set<Transaction*> toRemove;
        for (Transaction* buy : buys)
        {
            if (toRemove.count(buy)) continue;
            
            for (Transaction* sell : sells)
            {
                if (toRemove.count(sell)) continue;
                
                if (buy->GetPrice() >= sell->GetPrice())
                {
                    int tradeQty = std::min(buy->GetQuantity(), sell->GetQuantity()); 
                    
                    // FIX: Use trade price (seller's price) for both sides
                    int tradePrice = sell->GetPrice();
                    std::cout<< "TRADE" << " " << buy->GetOrderID() << " " << tradePrice << " " << tradeQty << " " << sell->GetOrderID() << " " << tradePrice << " " << tradeQty << std::endl;
                    
                    buy->UpdateQuantity(tradeQty);
                    sell->UpdateQuantity(tradeQty);
                    
                    if (buy->GetQuantity() == 0)
                    {
                        toRemove.insert(buy);
                    }
                    
                    if (sell->GetQuantity() == 0)
                    {
                        toRemove.insert(sell);
                    }
                    
                    if (buy->GetOrderType() == EOrderType::IOC)
                    {
                        toRemove.insert(buy);
                    }
                    
                    if (sell->GetOrderType() == EOrderType::IOC)
                    {
                        toRemove.insert(sell);
                    }
                    
                    if (toRemove.count(buy) || toRemove.count(sell))
                    {
                        break;
                    }
                }
            }
        }
        
        auto it = m_Transactions.begin();
        while (it != m_Transactions.end())
        {
            if (toRemove.count(*it))
            {
                delete *it;
                it = m_Transactions.erase(it);
            }
            else 
            {
                ++it;
            }
        }
        
        // Clear the matching in progress flag and notify waiting threads
        m_MatchingInProgress = false;
        m_MatchingDoneCV.notify_all();
    }
    
    void PrintTransaction()
    { 
        std::unique_lock<std::mutex> lock(m_Mutex);
        
        // FIX: Wait for matching to complete before printing to avoid inconsistent state
        m_MatchingDoneCV.wait(lock, [this] { return !m_MatchingInProgress; });
        
        std::map<int, int, std::greater<int>> sellBook;
        std::map<int, int, std::greater<int>> buyBook;
        
        for (Transaction* element : m_Transactions)
        {
            if (element->GetTransactionType() == ETransactType::SELL)
            {
                sellBook[element->GetPrice()] += element->GetQuantity();
            }
            else if (element->GetTransactionType() == ETransactType::BUY)
            {
                buyBook[element->GetPrice()] += element->GetQuantity();
            }
        }
        
        std::cout << "SELL:" << std::endl;
        for (const auto& [price, qty] : sellBook)
        {
            if (qty > 0)
            {
                std::cout<< price << " " << qty << std::endl;
            }
        }
        std::cout << "BUY:" << std::endl;
        for (const auto& [price, qty] : buyBook)
        {
            if (qty > 0)
            {
                std::cout<< price << " " << qty << std::endl;
            }
        }
    }

private:
    std::mutex m_Mutex;
    std::condition_variable m_CV;
    std::thread m_MatchTradeThread;
    std::atomic<bool> m_StopThread{false}; // FIX: Use atomic for thread-safe access
    
    std::vector<Transaction*> m_Transactions;
    std::atomic<bool> m_MatchingInProgress{false};
    std::condition_variable m_MatchingDoneCV;
};

// Helper functions
std::optional<int> ParseAsInt(const std::string& str)
{
    int value;
    auto [ptr, ec] = std::from_chars(str.data() , str.data() + str.size(), value);
    if (ec == std::errc() && ptr == str.data() + str.size())
    {
        return value;
    }
    return std::nullopt;
}

std::optional<EOrderType> ParseAsOrderType(const std::string& str)
{
    if (str == "IOC" || str == "ioc") return EOrderType::IOC;
    if (str == "GFD" || str == "gfd") return EOrderType::GFD;
    return std::nullopt;
}

std::optional<ETransactType> ParseAsTransactionType(const std::string& str)
{
    if (str == "BUY" || str == "buy") return ETransactType::BUY;
    if (str == "SELL" || str == "sell") return ETransactType::SELL;
    return std::nullopt;
}

int main() 
{  
    bool exit = false;
    TransactionMarket* market = new TransactionMarket();
  
    while(!exit)
    {
        std::string input, param1, param2, param3, param4, param5;
        std::getline(std::cin, input);
        std::istringstream iss(input);
        iss >> param1 >> param2 >> param3 >> param4 >> param5;
        
        if (param1 == "EXIT")
        { 
            if (market != nullptr)
            {
                delete market;
            }
            exit = true;
        }
        else if (param1 == "BUY" || param1 == "SELL")
        {
            std::optional<ETransactType> action = ParseAsTransactionType(param1);
            std::optional<EOrderType> type = ParseAsOrderType(param2);
            std::optional<int> price = ParseAsInt(param3);
            std::optional<int> quantity = ParseAsInt(param4);
            if (action.has_value() && type.has_value() && price.has_value() && quantity.has_value())
            {
                market->CreateTransaction(action.value(), type.value(), price.value(), quantity.value(), param5);
            }
        }
        else if (param1 == "CANCEL")
        {
            market->CancelTransaction(param2);
        }
        else if (param1 == "MODIFY")
        {
            std::optional<ETransactType> type = ParseAsTransactionType(param3);
            std::optional<int> price = ParseAsInt(param4);
            std::optional<int> quantity = ParseAsInt(param5);
            if (type.has_value() && price.has_value() && quantity.has_value())
            {
                // FIX: Pass quantity.value() instead of quantity.has_value()
                market->ModifyTransaction(param2, type.value(), price.value(), quantity.value());
            }
        }
        else if (param1 == "PRINT")
        {
            market->PrintTransaction();
        }
    }
  
    return 0;
}