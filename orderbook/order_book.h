#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <stack>
#include <list>

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

class OrderPool {
public:
    explicit OrderPool(size_t capacity) {
        orders_.resize(capacity);
        for (size_t i = 0; i < capacity; ++i)
            free_indices_.push(i);
    }

    Order* allocate() {
        if (free_indices_.empty()) return nullptr;
        size_t idx = free_indices_.top(); free_indices_.pop();
        return &orders_[idx];
    }

    void deallocate(Order* order_ptr) {
        size_t idx = order_ptr - &orders_[0];
        free_indices_.push(idx);
    }

private:
    std::vector<Order> orders_;
    std::stack<size_t> free_indices_;
};

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
    OrderPool pool_{200000};
};
