#include "Products.h"
#include <iostream>
#include <stdexcept>
#include <sstream>


Product::Product(int id, const std::string& n, double p, int q) 
    : product_id(id), name(n), price(p), stock_quantity(q) {
    if (price <= 0) {
        throw std::invalid_argument("Price must be positive");
    }
    if (stock_quantity < 0) {
        throw std::invalid_argument("Quantity can't be negative");
    }
}


std::unique_ptr<Product> Product::create(int id, const std::string& n, double p, int stock) { return std::make_unique<StandardProduct>(id, n, p, stock); }


std::unique_ptr<Product> Product::loadFromDb(int id, DatabaseConnection<std::string>& db) {
    try {
        std::string query = "SELECT product_id, name, price, stock_quantity FROM products WHERE product_id = " + 
                           std::to_string(id);
        auto result = db.executeQuery(query);
        
        if (result.empty() || result[0].empty()) {
            throw std::runtime_error("Product with ID: " + std::to_string(id) + " not found");
        }
        
        int product_id = std::stoi(result[0][0]);
        std::string name = result[0][1];
        double price = std::stod(result[0][2]);
        int stock = std::stoi(result[0][3]);
        
        return std::make_unique<StandardProduct>(product_id, name, price, stock);
    } catch (const std::exception& e) {
        std::cerr << "Error with load: " << e.what() << "\n";
        return nullptr;
    }
}


void Product::decreaseStock(int quantity) {
    if (stock_quantity < quantity) {
        throw std::runtime_error("Not enough products");
    }
    stock_quantity -= quantity;
}

void Product::increaseStock(int quantity) {
    if (quantity <= 0) {
        throw std::invalid_argument("Quantity must be pos");
    }
    stock_quantity += quantity;
}

void Product::setPrice(double new_price) {
    if (new_price <= 0) {
        throw std::invalid_argument("Price must be pos");
    }
    price = new_price;
}

void Product::setStockQuantity(int new_quantity) {
    if (new_quantity < 0) {
        throw std::invalid_argument("Quantity can't be negative");
    }
    stock_quantity = new_quantity;
}


bool Product::validate() const { return !name.empty() && price > 0 && stock_quantity >= 0; }


void Product::updateInDatabase(DatabaseConnection<std::string>& db) {
    if (!validate()) {
        throw std::runtime_error("Product isn't validate");
    }
    
    try {
        std::string check_query = "SELECT COUNT(*) FROM products WHERE product_id = " + 
                                 std::to_string(product_id);
        auto result = db.executeQuery(check_query);
        
        if (!result.empty() && !result[0].empty() && std::stoi(result[0][0]) > 0) {
            std::string update_query = 
                "UPDATE products SET name = '" + name + "', " +
                "price = " + std::to_string(price) + ", " +
                "stock_quantity = " + std::to_string(stock_quantity) + " " +
                "WHERE product_id = " + std::to_string(product_id);
            db.executeNonQuery(update_query);
        } else {
            std::string insert_query = 
                "INSERT INTO products (name, price, stock_quantity) VALUES ('" +
                name + "', " + std::to_string(price) + ", " +
                std::to_string(stock_quantity) + ") RETURNING product_id";
            
            auto insert_result = db.executeQuery(insert_query);
            if (!insert_result.empty() && !insert_result[0].empty()) {
                product_id = std::stoi(insert_result[0][0]);
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error with update in database: " + std::string(e.what()));
    }
}


void Product::saveToDatabase(DatabaseConnection<std::string>& db, bool is_update) {
    std::string query;
    
    if (is_update) {
        query = "UPDATE products SET name = '" + name + 
                "', price = " + std::to_string(price) + 
                ", stock_quantity = " + std::to_string(stock_quantity) + 
                " WHERE product_id = " + std::to_string(product_id);
    } else {
        query = "INSERT INTO products (name, price, stock_quantity) VALUES ('" + 
                name + "', " + std::to_string(price) + ", " + 
                std::to_string(stock_quantity) + ")";
    }
    
    db.executeNonQuery(query);
}


std::string StandardProduct::getProductInfo() const {
    std::stringstream ss;
    ss << "Product: " << name 
       << " (ID: " << product_id 
       << "), Price: " << price 
       << " rub., Stock: " << stock_quantity;
    return ss.str();
}


bool StandardProduct::validate() const {
    bool base_valid = Product::validate();
    bool name_valid = name.length() >= 2 && name.length() <= 100;
    bool price_valid = price >= 10.0 && price <= 1000000.0;
    return base_valid && name_valid && price_valid;
}


void StandardProduct::updateInDatabase(DatabaseConnection<std::string>& db) {
    if (!validate()) {
        throw std::runtime_error("Product isn't validate");
    }
    
    try {
        TransactionGuard guard(db);
        
        std::string check_query = "SELECT product_id FROM products WHERE product_id = " + 
                                 std::to_string(product_id) + " FOR UPDATE";
        auto result = db.executeQuery(check_query);
        
        if (!result.empty()) {
            saveToDatabase(db, true);
        } else {
            saveToDatabase(db, false);
            auto id_result = db.executeQuery("SELECT LASTVAL()");
            if (!id_result.empty() && !id_result[0].empty()) {
                product_id = std::stoi(id_result[0][0]);
            }
        }
        
        guard.commit();
        std::cout << "Product saves to db: " << name << "\n";
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Error with saving: " + std::string(e.what()));
    }
}