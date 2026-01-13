#include "User.h"
#include "Order.h"
#include "Database.h"
#include "Products.h"
#include "AuditManager.h"
#include <iostream>
#include <memory>
#include <algorithm>
#include <stdexcept>


std::shared_ptr<Order> Admin::createOrder() {
    std::cerr << "Admin isn't creating orders\n";
    return nullptr;
}


std::shared_ptr<Order> Manager::createOrder() {
    std::cerr << "Manager isn't creating orders\n";
    return nullptr;
}


std::string Manager::viewOrderStatus(int order_id) const {
    return "Status is preparing";
}


bool Manager::cancelOrder(int order_id) {
    std::cout << "Manager canceled order #" << order_id << "\n";
    AuditManager::getLogger()->logAudit("order", order_id, "update", 
                                        "Manager canceled order #" + std::to_string(order_id));
    return true;
}


void Admin::viewAllOrders() {
    std::cout << "Admin is viewing all orders\n";
}


void Admin::updateOrderStatus(int order_id, const std::string& new_status) {
    std::cout << "Admin is updating order status #" << order_id 
              << " to '" << new_status << "'\n";
    AuditManager::logStatusChange(order_id, "unknown", new_status);
}


std::shared_ptr<Order> Customer::createOrder() {
    try {
        static int order_counter = 1000;
        int new_order_id = ++order_counter;
        
        auto order = std::make_shared<Order>(new_order_id, getUserId());
        addOrder(order);
        
        std::cout << "Customer created new order #" << new_order_id << "\n";
        AuditManager::getLogger()->logAudit("order", new_order_id, "insert", "Order has been created#" + std::to_string(getUserId()));
        
        return order;
    } catch (const std::exception& e) {
        std::cerr << "Error with create order: " << e.what() << "\n";

        return nullptr;
    }
}


std::string Customer::viewOrderStatus(int order_id) const {
    auto it = std::find_if(userOrders.begin(), userOrders.end(),
        [order_id](const std::shared_ptr<Order>& order) {
            return order->getOrderId() == order_id;
        });
    
    if (it != userOrders.end()) {
        return "Status of order #" + std::to_string(order_id) + 
               ": " + (*it)->getStatus();
    }
    
    return "Order #" + std::to_string(order_id) + " no found";
}


bool Customer::cancelOrder(int order_id) {
    auto it = std::find_if(userOrders.begin(), userOrders.end(),
        [order_id](const std::shared_ptr<Order>& order) {
            return order->getOrderId() == order_id;
        });
    
    if (it != userOrders.end() && (*it)->cancel()) {
        AuditManager::getLogger()->logAudit("order", order_id, "update", "Customer canceled order");
        return true;
    }
    
    std::cerr << "Error with canceling order#" << order_id << "\n";
    return false;
}


void Admin::addProduct(const std::string& name, double price, int quantity) {
    try {
        auto product = Product::create(0, name, price, quantity);
        std::cout << "Admin added product: " << product->getProductInfo() << "\n";
        
        AuditManager::logProductChange(0, "insert", "Added: " + name);
    } catch (const std::exception& e) {
        std::cerr << "Error with add: " << e.what() << "\n";
    }
}


void Manager::approveOrder(int order_id) {
    std::cout << "Manager approved order ID:" << order_id << "\n";
    AuditManager::logOrderChange(order_id, "update", "Approved by manager");
}


void Manager::updateStock(int product_id, int new_quantity) {
    std::cout << "Manager is updating stock quantity Id" << product_id 
              << " to " << new_quantity;
    AuditManager::logProductChange(product_id, "update", "Update stocks: " + std::to_string(new_quantity));
}


void Customer::addToOrder(int order_id, int product_id, int quantity) {
    auto it = std::find_if(userOrders.begin(), userOrders.end(),
        [order_id](const std::shared_ptr<Order>& order) {
            return order->getOrderId() == order_id;
        });
    
    if (it != userOrders.end()) {
        try {
            auto item = std::make_unique<OrderItem>(product_id, quantity, 0.0);
            (*it)->addItem(std::move(item));
            std::cout << "Product #" << product_id << " added to order #" << order_id << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Error with add to order: " << e.what() << "\n";
        }
    } else {
        std::cerr << "Order Id:" << order_id << " not found\n";
    }
}


bool Customer::makePayment(int order_id, const std::string& payment_method) {
    std::cout << "Customer is paying #" << order_id 
              << " method: " << payment_method << "\n";
    return true;
}


void Admin::showInfo() const {
    std::cout << "Admin: " << getName() << " (ID: " << getUserId() 
              << "), loyalty level: " << (getLoyaltyLevel() ? "Prem" : "Bas") << "\n";
}


void Manager::showInfo() const {
    std::cout << "Manager: " << getName() << " (ID: " << getUserId() 
              << "), loyalty level: " << (getLoyaltyLevel() ? "Prem" : "Bas") << "\n";
}


std::string Admin::viewOrderStatus(int order_id) const {
    try {
        return "Status in admining";
    } catch (...) {
        return "Error with obtaining the status";
    }
}


bool Admin::cancelOrder(int order_id) {
    std::cout << "Admin canceled #" << order_id << "\n";
    AuditManager::getLogger()->logAudit("order", order_id, "update", 
                                        "Admin canceled order#" + std::to_string(order_id));
    return true;
}


void Customer::showInfo() const {
    std::cout << "Customer: " << getName() << " (ID: " << getUserId() 
              << "), loyalty level: " << (getLoyaltyLevel() ? "Prem" : "Bas") 
              << ", amount of orders: " << userOrders.size() << "\n";
}


void Customer::removeFromOrder(int order_id, int product_id) {
    std::cout << "Customer removing product Id:" << product_id << "\n";
}


void Admin::updateProduct(int product_id, const std::string& name, double price, int quantity) {
    std::cout << "Admin updating product Id:" << product_id << "\n";
}


void Admin::deleteProduct(int product_id) {
    std::cout << "Admin deleting product Id:" << product_id << "\n";
}