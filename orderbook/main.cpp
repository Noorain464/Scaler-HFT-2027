#include "order_book.h"
#include <iostream>
#include <chrono>
#include <random>

int main() {
    using namespace std::chrono;
    OrderBook ob;

    const int N = 100000; // number of orders
    std::vector<uint64_t> order_ids;
    order_ids.reserve(N);

    std::mt19937 rng(42); // random generator
    std::uniform_real_distribution<double> price_dist(90.0, 110.0);
    std::uniform_int_distribution<int> side_dist(0, 1);
    std::uniform_int_distribution<uint64_t> qty_dist(1, 1000);

    auto now_ns = [](){
        using namespace std::chrono;
        return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
    };

    // ---------- Add Orders ----------
    auto t1 = high_resolution_clock::now();
    for (uint64_t i = 1; i <= N; ++i) {
        Order o;
        o.order_id = i;
        o.is_buy = side_dist(rng) == 1;
        o.price = price_dist(rng);
        o.quantity = qty_dist(rng);
        o.timestamp_ns = now_ns();

        ob.add_order(o);
        order_ids.push_back(i);
    }
    auto t2 = high_resolution_clock::now();
    std::cout << "Add " << N << " orders took: "
              << duration_cast<milliseconds>(t2 - t1).count() << " ms\n";

    // ---------- Amend Orders ----------
    auto t3 = high_resolution_clock::now();
    for (int i = 0; i < N; i += 10) { // amend every 10th order
        double new_price = price_dist(rng);
        uint64_t new_qty = qty_dist(rng);
        ob.amend_order(order_ids[i], new_price, new_qty);
    }
    auto t4 = high_resolution_clock::now();
    std::cout << "Amend " << N/10 << " orders took: "
              << duration_cast<milliseconds>(t4 - t3).count() << " ms\n";

    // ---------- Cancel Orders ----------
    auto t5 = high_resolution_clock::now();
    for (int i = 0; i < N; i += 5) { // cancel every 5th order
        ob.cancel_order(order_ids[i]);
    }
    auto t6 = high_resolution_clock::now();
    std::cout << "Cancel " << N/5 << " orders took: "
              << duration_cast<milliseconds>(t6 - t5).count() << " ms\n";

    return 0;
}
