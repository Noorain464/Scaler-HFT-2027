#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <stack>
#include <list>
#include <new>
#include <type_traits>

// --------------------------------------
// Generic MemoryPool
// --------------------------------------
template<typename T, size_t BlockSize = 4096>
class MemoryPool {
private:
    struct Block {
        static constexpr size_t num_objects = BlockSize / sizeof(T);
        typename std::aligned_storage<sizeof(T), alignof(T)>::type data[num_objects];
        Block* next;
    };

    Block* current_block_;
    size_t current_offset_;
    std::vector<Block*> all_blocks_;
    std::vector<T*> free_list_;

public:
    MemoryPool() : current_block_(nullptr), current_offset_(0) {
        allocate_new_block();
    }

    ~MemoryPool() {
        for (auto* block : all_blocks_)
            delete block;
    }

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    template<typename... Args>
    T* construct(Args&&... args) {
        T* ptr = allocate();
        new (ptr) T(std::forward<Args>(args)...);
        return ptr;
    }

    void destroy(T* ptr) {
        if (ptr) {
            ptr->~T();
            free_list_.push_back(ptr);
        }
    }

    T* allocate() {
        if (!free_list_.empty()) [[unlikely]] {
            T* ptr = free_list_.back();
            free_list_.pop_back();
            return ptr;
        }
        if (current_block_ == nullptr || current_offset_ >= Block::num_objects) [[unlikely]] {
            allocate_new_block();
        }
        T* ptr = reinterpret_cast<T*>(&current_block_->data[current_offset_]);
        ++current_offset_;
        return ptr;
    }

    void deallocate(T* ptr) {
        free_list_.push_back(ptr);
    }

private:
    void allocate_new_block() {
        Block* new_block = new Block();
        new_block->next = nullptr;
        if (current_block_) current_block_->next = new_block;
        all_blocks_.push_back(new_block);
        current_block_ = new_block;
        current_offset_ = 0;
    }
};

// --------------------------------------
// Order Structures
// --------------------------------------
struct Order {
    uint64_t order_id;
    bool is_buy;           
    double price;
    uint64_t quantity;
    uint64_t timestamp_ns;
};

struct PriceLevel {
    double price;
    uint64_t total_quantity;
};

// --------------------------------------
// OrderBook class
// --------------------------------------
class OrderBook {
public:
    void add_order(const Order& order);
    bool cancel_order(uint64_t order_id);
    bool amend_order(uint64_t order_id, double new_price, uint64_t new_quantity);
    void get_snapshot(size_t depth, std::vector<PriceLevel>& bids, std::vector<PriceLevel>& asks) const;
    void print_book(size_t depth = 10) const;

private:
    std::map<double, std::list<Order*>, std::greater<double>> bids_;
    std::map<double, std::list<Order*>, std::less<double>> asks_;
    std::unordered_map<uint64_t, Order*> order_lookup_;
    MemoryPool<Order, 4096 * 10> pool_; 
};
