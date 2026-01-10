#pragma once


#include "Database.h"
#include <string>
#include <memory>


class Product{
    private:
        int product_id;
        std::string name;
        double price;
        int stock_quantity;

    public:
        Product(int id, const std::string& n, double p, int q);
        virtual ~Product();

        static std::shared_ptr<Product> loadFromDb(int id, DatabaseConnection<std::string>& db);
        static std::unique_ptr<Product> create(int id, const std::string& n, double p, int stock);

        void decreaseStockQuantity();
        void deliveryProduct();
        void saveToDatabase(DatabaseConnection<std::string>& db);

        std::string getName() const;
        double getPrice() const;
        int getStockquantity() const;

        void setName(const std::string& newname);
        void setPrice(double newprice);
        void setStockquantity(int newquantity);
};