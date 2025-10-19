#include "order_book.h"
#include <algorithm>

void OrderBook::add_order(const Order& order) {
    Order* o = pool_.construct(order); 
    if (!o) {
        std::cerr << "Order pool full!\n";
        return;
    }

    if (order.is_buy) {
        bids_[order.price].push_back(o);
    } else {
        asks_[order.price].push_back(o);
    }
    order_lookup_[order.order_id] = o;
}

bool OrderBook::cancel_order(uint64_t order_id) {
    auto it_lookup = order_lookup_.find(order_id);
    if (it_lookup == order_lookup_.end()) return false;

    Order* order_ptr = it_lookup->second;

    if (order_ptr->is_buy) {
        auto it_price = bids_.find(order_ptr->price);
        if (it_price != bids_.end()) {
            it_price->second.remove(order_ptr);
            if (it_price->second.empty()) bids_.erase(it_price);
        }
    } else {
        auto it_price = asks_.find(order_ptr->price);
        if (it_price != asks_.end()) {
            it_price->second.remove(order_ptr);
            if (it_price->second.empty()) asks_.erase(it_price);
        }
    }

    order_lookup_.erase(it_lookup);
    pool_.destroy(order_ptr);  
    return true;
}

bool OrderBook::amend_order(uint64_t order_id, double new_price, uint64_t new_quantity) {
    auto it = order_lookup_.find(order_id);
    if (it == order_lookup_.end()) return false;

    Order* order_ptr = it->second;

    if (order_ptr->price == new_price) {
        order_ptr->quantity = new_quantity;
    } else {
        Order updated_order = *order_ptr;
        cancel_order(order_id);
        updated_order.price = new_price;
        updated_order.quantity = new_quantity;
        add_order(updated_order);
    }
    return true;
}

void OrderBook::get_snapshot(size_t depth,
                             std::vector<PriceLevel>& bids,
                             std::vector<PriceLevel>& asks) const {
    bids.clear();
    asks.clear();

    // Aggregate bids
    for (auto it = bids_.begin(); it != bids_.end() && bids.size() < depth; ++it) {
        uint64_t total_qty = 0;
        for (const auto* order_ptr : it->second)
            total_qty += order_ptr->quantity;
        bids.push_back({it->first, total_qty});
    }

    // Aggregate asks
    for (auto it = asks_.begin(); it != asks_.end() && asks.size() < depth; ++it) {
        uint64_t total_qty = 0;
        for (const auto* order_ptr : it->second)
            total_qty += order_ptr->quantity;
        asks.push_back({it->first, total_qty});
    }
}

void OrderBook::print_book(size_t depth) const {
    std::vector<PriceLevel> bids, asks;
    get_snapshot(depth, bids, asks);

    std::cout << "----- ORDER BOOK (Top " << depth << ") -----\n";
    std::cout << "   BID_QTY    BID_PRICE | ASK_PRICE    ASK_QTY\n";
    std::cout << "-----------------------------------------------\n";

    for (size_t i = 0; i < depth; ++i) {
        std::string bid_str = (i < bids.size())
                                  ? std::to_string(bids[i].total_quantity) + " @ " + std::to_string(bids[i].price)
                                  : "";
        std::string ask_str = (i < asks.size())
                                  ? std::to_string(asks[i].price) + " @ " + std::to_string(asks[i].total_quantity)
                                  : "";
        std::cout << "  " << bid_str << "  |  " << ask_str << "\n";
    }
}
