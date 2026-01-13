#include "Order.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <iostream>


Order::Order(int id, int uid) 
    : order_id(id), user_id(uid), status("pending"), total_price(0.0),
      order_date(std::chrono::system_clock::now()) {
    if (id <= 0) throw std::invalid_argument("ID order must be pos");
    if (uid <= 0) throw std::invalid_argument("ID must be pos");
}


Order::~Order() = default;


void Order::addItem(std::unique_ptr<OrderItem> item) {
    if (!item) {
        throw std::invalid_argument("Invalid el");
    }
    
    auto it = std::find_if(items.begin(), items.end(),
        [&item](const std::unique_ptr<OrderItem>& existing) {
            return existing->getProductId() == item->getProductId();
        });
    
    if (it != items.end()) {
        (*it)->setQuantity((*it)->getQuantity() + item->getQuantity());
    } else {
        items.push_back(std::move(item));
    }
    
    calculateTotal();
}


void Order::removeItem(int product_id) {
    if (product_id <= 0) {
        throw std::invalid_argument("Invalid ID");
    }
    
    auto initial_size = items.size();
    items.erase(
        std::remove_if(items.begin(), items.end(),
            [product_id](const std::unique_ptr<OrderItem>& item) {
                return item->getProductId() == product_id;
            }),
        items.end()
    );
    
    if (items.size() < initial_size) {
        calculateTotal();
        std::cout << "Product ID " << product_id << " deleted from order\n";
    } else {
        std::cout << "Product ID " << product_id << " is no found in order\n";
    }
}


void Order::updateStatus(const std::string& new_status) {
    std::vector<std::string> valid_statuses = {"pending", "completed", "canceled", "returned"};
    
    if (std::find(valid_statuses.begin(), valid_statuses.end(), new_status) == valid_statuses.end()) {
        throw std::invalid_argument("Invalid status: " + new_status);
    }
    
    std::string old_status = status;
    status = new_status;
    std::cout << "Order status #" << order_id << " replaced by: " 
              << old_status << " -> " << new_status << "\n";
}


void Order::calculateTotal() { total_price = calculateItemsTotal(); }


void Order::setPayment(std::unique_ptr<Payment> p) {
    if (!p) {
        throw std::invalid_argument("Invalid arg");
    }
    
    payment = std::move(p);
    std::cout << "Payment set to order #" << order_id << "\n";
}


bool Order::processPayment() {
    if (!payment) {
        std::cerr << "Error with payment #" << order_id << "\n";
        return false;
    }
    
    if (payment->isPaid()) {
        std::cout << "Payment active #" << order_id << "\n";
        return true;
    }
    
    bool success = payment->process();
    if (success) {
        updateStatus("completed");
    }
    
    return success;
}


bool Order::cancel() {
    if (status == "pending") {
        updateStatus("canceled");
        
        std::cout << "Order #" << order_id << " canceled\n";
        return true;
    }
    
    std::cerr << "Can't cancel the order #" << order_id 
              << ". Status: " << status << "\n";
    return false;
}


bool Order::returnOrder() {
    if (status == "completed") {
        updateStatus("returned");
        std::cout << "Order #" << order_id << " ready to return.\n";
        
        if (payment && payment->isPaid()) {
            std::cout << "Price of product will return in few days.\n";
        }
        
        return true;
    }
    
    std::cerr << "Can't return order #" << order_id 
              << ". Status: " << status << "\n";
    return false;
}


double Order::calculateItemsTotal() const {
    return std::accumulate(items.begin(), items.end(), 0.0,
        [](double sum, const std::unique_ptr<OrderItem>& item) {
            return sum + item->getTotal();
        });
}


auto filterByStatus = [](const std::unique_ptr<Order>& order, const std::string& target_status) { return order->getStatus() == target_status; };