#pragma once


#include <string>
#include <memory>


class Order;


class User{
    protected:
        int user_id;
        std::string name;
        std::string email;
        std::string role;
        std::string password_hash;
        bool loyalty_level;

    public:
        User(int i, const std::string& n, const std::string& e, const std::string& r, const std::string& password, bool l) :
        user_id(i), name(n), email(e), role(r), password_hash(password), loyalty_level(l) {
            if (role != "admin" && role != "manager" && role != "customer") role = "customer";
        }

        virtual std::shared_ptr<Order> createOrder();
        virtual std::string viewOrderStatus(int order_id) const;
        virtual bool cancelOrder(int order_id);

        virtual ~User();
        virtual void showInfo() const = 0;

        int getUserid() const { return user_id; }
        std::string getName() const { return name; }
        std::string getEmail() const { return email; }
        std::string getRole() const { return role; }
        std::string getPasswordhash() const { return password_hash; }
        bool getLoyaltylevel() const { return loyalty_level; }

        void setName(const std::string& n) { name = n; }
        void setEmail(const std::string& e) { email = e; }
        void setLoyaltylevel(bool level) { loyalty_level = level; }
};


class Admin : public User{
    public:
        Admin(int i, const std::string& n, const std::string& e, const std::string& password, bool l) : User(i, n, e, "admin", password, l) {};

        std::shared_ptr<Order> createOrder() override { return nullptr; }

        void addProduct();
        void updateProduct();
        void deleteProduct();
        void viewAllOrders();
        void updateOrderStatus();
};


class Manager : public User{
    public:
        Manager(int i, const std::string& n, const std::string& e, const std::string& password, bool l) : User(i, n, e, "manager", password, l) {};

        void approveOrder();
        void updateStock();
};


class Customer : public User{
    public:
        Customer(int i, const std::string& n, const std::string& e, const std::string& password, bool l) : User(i, n, e, "customer", password, l) {};

        void addToOrder();
        void removeFromOrder();
        void makePayment();
};