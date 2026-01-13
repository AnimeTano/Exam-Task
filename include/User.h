#pragma once


#include <string>
#include <memory>
#include <vector>
#include <algorithm>


class Order;


class User {
    protected:
        int user_id;
        std::string name;
        std::string email;
        std::string role;
        std::string password_hash;
        bool loyalty_level;
        std::vector<std::shared_ptr<Order>> userOrders;

    public:
        User(int i, const std::string& n, const std::string& e, 
            const std::string& r, const std::string& password, bool l) :
            user_id(i), name(n), email(e), role(r), password_hash(password), loyalty_level(l) {
            if (role != "admin" && role != "manager" && role != "customer") role = "customer";
        }

        virtual ~User() {}
        
        virtual void showInfo() const = 0;
        
        virtual std::shared_ptr<Order> createOrder() = 0;
        virtual std::string viewOrderStatus(int order_id) const = 0;
        virtual bool cancelOrder(int order_id) = 0;

        void addOrder(std::shared_ptr<Order> order) { userOrders.push_back(order); }
        void removeOrder(int order_id) {}

        const std::vector<std::shared_ptr<Order>>& getOrders() const { return userOrders; }

        int getUserId() const { return user_id; }
        std::string getName() const { return name; }
        std::string getEmail() const { return email; }
        std::string getRole() const { return role; }
        std::string getPasswordHash() const { return password_hash; }
        bool getLoyaltyLevel() const { return loyalty_level; }
};


class Admin : public User {
    public:
        Admin(int i, const std::string& n, const std::string& e, 
            const std::string& password, bool l) : User(i, n, e, "admin", password, l) {}
        
        void showInfo() const override;

        std::shared_ptr<Order> createOrder() override;
        std::string viewOrderStatus(int order_id) const override;
        bool cancelOrder(int order_id) override;

        void addProduct(const std::string& name, double price, int quantity);
        void updateProduct(int product_id, const std::string& name, double price, int quantity);
        void deleteProduct(int product_id);
        void viewAllOrders();
        void updateOrderStatus(int order_id, const std::string& new_status);
};


class Manager : public User {
    public:
        Manager(int i, const std::string& n, const std::string& e, 
                const std::string& password, bool l) : User(i, n, e, "manager", password, l) {}
        
        void showInfo() const override;

        std::shared_ptr<Order> createOrder() override;
        std::string viewOrderStatus(int order_id) const override;
        bool cancelOrder(int order_id) override;

        void approveOrder(int order_id);
        void updateStock(int product_id, int new_quantity);
};


class Customer : public User {
    public:
        Customer(int i, const std::string& n, const std::string& e, 
                const std::string& password, bool l) : User(i, n, e, "customer", password, l) {}
        
        void showInfo() const override;

        std::shared_ptr<Order> createOrder() override;
        std::string viewOrderStatus(int order_id) const override;
        bool cancelOrder(int order_id) override;

        void addToOrder(int order_id, int product_id, int quantity);
        void removeFromOrder(int order_id, int product_id);
        bool makePayment(int order_id, const std::string& payment_method);
};