#include "User.h"
#include <iostream>


void Admin::showInfo() const {
    std::cout << "Admin: " << getName() << " (ID: " << getUserId() << ")\n";
}

void Manager::showInfo() const {
    std::cout << "Manager: " << getName() << " (ID: " << getUserId() << ")\n";
}

void Customer::showInfo() const {
    std::cout << "Customer: " << getName() << " (ID: " << getUserId() << ")\n";
}


void Admin::addProduct() {
    std::cout << "Admin adding product\n";
}


void Admin::updateProduct() {
    std::cout << "Admin updating product\n";
}


void Admin::updateOrderStatus() {
    std::cout << "Admin updating order status\n";
}

void Manager::updateStock() {
    std::cout << "Manager updating stock\n";
}


void Admin::deleteProduct() {
    std::cout << "Admin deleting product\n";
}


void Admin::viewAllOrders() {
    std::cout << "Admin viewing all orders\n";
}


void Manager::approveOrder() {
    std::cout << "Manager approving order\n";
}


void Customer::addToOrder() {
    std::cout << "Customer adding to order\n";
}

void Customer::removeFromOrder() {
    std::cout << "Customer removing from order\n";
}

void Customer::makePayment() {
    std::cout << "Customer making payment\n";
}