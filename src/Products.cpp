#include "Products.h"
#include <iostream>


Product::Product(int id, const std::string& n, double p, int q) 
    : product_id(id), name(n), price(p), stock_quantity(q) {
    if (price < 0) price = 0.0;
}


Product::~Product() {}


std::shared_ptr<Product> Product::loadFromDb(int id, DatabaseConnection<std::string>& db) {
    return std::make_shared<Product>(id, "Test Product", 100.0, 10);
}


std::unique_ptr<Product> Product::create(int id, const std::string& n, double p, int stock) {
    return std::make_unique<Product>(id, n, p, stock);
}


void Product::decreaseStockQuantity() {
    if (stock_quantity > 0) stock_quantity--;
}


void Product::deliveryProduct() {
    std::cout << "Product delivered: " << name << "\n";
}


void Product::saveToDatabase(DatabaseConnection<std::string>& db) {
    std::string query = "INSERT INTO products (name, price, stock_quantity) VALUES ('" +
                       name + "', " + std::to_string(price) + ", " + 
                       std::to_string(stock_quantity) + ")";
    try {
        db.executeNonQuery(query);
        std::cout << "Product saved to database\n";
    } catch (...) {
        std::cerr << "Failed to save product\n";
    }
}


std::string Product::getName() const { return name; }
double Product::getPrice() const { return price; }
int Product::getStockquantity() const { return stock_quantity; }

void Product::setName(const std::string& newname) { name = newname; }
void Product::setPrice(double newprice) { 
    if (newprice < 0) price = 0.0; 
    else price = newprice;
}
void Product::setStockquantity(int newquantity) { stock_quantity = newquantity; }