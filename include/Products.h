#pragma once


#include "Database.h"
#include <string>
#include <memory>
#include <iostream>


class Product {
    protected:
        int product_id;
        std::string name;
        double price;
        int stock_quantity;

    protected:
        void saveToDatabase(DatabaseConnection<std::string>& db, bool is_update = false);

    public:
        Product(int id, const std::string& n, double p, int q);
        virtual ~Product() = default;

        static std::unique_ptr<Product> loadFromDb(int id, DatabaseConnection<std::string>& db);
        static std::unique_ptr<Product> create(int id, const std::string& n, double p, int stock);

        virtual std::string getProductInfo() const = 0;
        virtual void updateInDatabase(DatabaseConnection<std::string>& db);
        virtual bool validate() const;

        void decreaseStock(int quantity);
        void increaseStock(int quantity);

        int getId() const { return product_id; }
        std::string getName() const { return name; }
        double getPrice() const { return price; }
        int getStockquantity() const { return stock_quantity; }

        void setName(const std::string& new_name) { name = new_name; }
        void setPrice(double new_price);
        void setStockQuantity(int newquantity);
};


class StandardProduct : public Product {
    public:
        StandardProduct(int id, const std::string& n, double p, int q) 
            : Product(id, n, p, q) {}
        
        std::string getProductInfo() const override;
        
        void updateInDatabase(DatabaseConnection<std::string>& db) override;
        bool validate() const override;
};