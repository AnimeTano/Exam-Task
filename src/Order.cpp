#include "Order.h"
#include "Payment.h"
#include <algorithm>


Order::Order(int id, int uid) 
    : order_id(id), user_id(uid), status("pending"), total_price(0),
      order_date(std::chrono::system_clock::now()) {}

void Order::addItem(const OrderItem& item) {
    items.push_back(item);
}


void Order::removeItem(int product_id) {
    items.erase(
        std::remove_if(items.begin(), items.end(),
            [product_id](const OrderItem& item) {
                return item.getProductId() == product_id;
            }),
        items.end()
    );
}


void Order::updateStatus(const std::string& newstatus) {
    status = newstatus;
}


void Order::calculateTotal() {
    total_price = 0;
    for (const auto& item : items) {
        total_price += item.getTotal();
    }
}


void Order::setPayment(std::unique_ptr<Payment> p) {
    payment = std::move(p);
}


bool Order::cancel() {
    if (status == "pending") {
        status = "canceled";
        return true;
    }
    return false;
}


bool Order::returnOrder() {
    if (status == "completed") {
        status = "returned";
        return true;
    }
    return false;
}