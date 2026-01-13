#pragma once


#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include "Payment.h"


class OrderItem {
    private:
        int product_id;
        int quantity;
        double price;
        
    public:
        OrderItem(int id, int q, double p) : product_id(id), quantity(q), price(p) {
            if (quantity <= 0) throw "Quantity must be positive";
            if (price < 0) throw "Price cannot be negative";
        }

        int getProductId() const { return product_id; }
        int getQuantity() const { return quantity; }
        double getPrice() const { return price; }
        double getTotal() const { return quantity * price; }

        void setQuantity(int q) { quantity = q; }
        void setPrice(double p) { price = p; }
};


class Payment;


class Order {
    private:
        int order_id;
        int user_id;
        std::string status;
        double total_price;
        std::chrono::system_clock::time_point order_date;
        std::vector<std::unique_ptr<OrderItem>> items;
        std::unique_ptr<Payment> payment;

    public:
        Order(int id, int uid);
        ~Order();
        
        void removeItem(int product_id);
        void addItem(std::unique_ptr<OrderItem> item);
        void updateStatus(const std::string& newstatus);
        void calculateTotal();
        void setPayment(std::unique_ptr<Payment> p);
        bool processPayment();

        bool cancel();
        bool returnOrder();

        double calculateItemsTotal() const;

        template<typename Predicate>
        std::vector<OrderItem*> filterItems(Predicate predicate) const {
            std::vector<OrderItem*> res;

            for (const auto& i : items) {
                if (predicate(i.get())) {
                    res.push_back(i.get());
                }
            }

            return res;
        }

        int getOrderId() const { return order_id; }
        int getUserId() const { return user_id; }
        std::string getStatus() const { return status; }
        double getTotalPrice() const { return total_price; }
        auto getOrderDate() const { return order_date; }
        const std::vector<std::unique_ptr<OrderItem>>& getItems() const { return items; }
        bool hasPayment() const { return payment != nullptr; }
        Payment* getPayment() const { return payment.get(); }
};