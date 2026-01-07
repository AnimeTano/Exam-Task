#pragma once


#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include "Payment.h"


class OrderItem{
    private:
        int product_id;
        int quantity;
        double price;
    
    public:
        OrderItem(int id, int q, double p) : product_id(id), quantity(q), price(p) {
            if (quantity <= 0) throw "Quantity can't be negative";
            if (price <= 0) throw "Price can't be negative";
        };

        int getProductId() const { return product_id; }
        int getQuantity() const { return quantity; }
        double getPrice() const { return price; }

        void setQuantity(int q) { quantity = q; }
};


class Order{
    private:
        int order_id;
        int user_id;
        std::string status;
        double total_price;
        std::chrono::system_clock::time_point order_date;
        std::vector<OrderItem> items;
        std::unique_ptr<Payment> payment;

    public:
        Order(int id, int user_id);

        void removeItem(int product_id);
        void addItem(const OrderItem& item);
        void updateStatus(const std::string& newstatus);
        void calculateTotal();
        void setPayment(std::unique_ptr<Payment> payment);

        bool cancel();
        bool returnOrder();

        int getOrderid() const { return order_id; }
        int getUserid() const { return user_id; }
        std::string getStatus() const { return status; }
        double getTotalPrice() const { return total_price; }
        auto getOrderDate() const { return order_date; }
        std::vector<OrderItem> getItems() const { return items; }
        Payment* getPayment() const { return payment.get(); }
};