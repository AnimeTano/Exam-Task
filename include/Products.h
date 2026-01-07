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
        Product(int id, const std::string& n, double p, int q) : product_id(id), name(n), price(p), stock_quantity(q) {
            if (price < 0) price = 0.0;
        };

        virtual ~Product() = default;

        static std::shared_ptr<Product> loadFromDb(int id, DatabaseConnection<std::string>& db);
        static std::unique_ptr<Product> create(int id, const std::string& n, double p, int stock);

        void decreaseStockQuantity();
        void deliveryProduct();
        void saveToDatabase(DatabaseConnection<std::string>& db);

        std::string getName() const { return name; }
        double getPrice() const { return price; }
        int getStockquantity() const { return stock_quantity; }

        void setName(const std::string& newname) { name = newname; }
        void setPrice(double newprice) { if (newprice < 0) price = 0.0; }
        void setStockquantity(int newquantity) { stock_quantity = newquantity; }
};