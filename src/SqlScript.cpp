#include "SqlScript.h"
#include <iostream>


bool SqlScript::executeScript(DatabaseConnection<>& db, const std::string& filepath) { return true; }
bool SqlScript::executeAllScripts(DatabaseConnection<>& db, const std::string& directory) { return true; }

bool SqlScript::initializeDatabase(DatabaseConnection<>& db, const std::string& sqlDir) {
    std::cout << "\nCreate tabels" << "\n";
    
    try {
        db.executeNonQuery(
            "CREATE TABLE IF NOT EXISTS users ("
            "user_id SERIAL PRIMARY KEY, "
            "name VARCHAR(100) NOT NULL, "
            "email VARCHAR(70) NOT NULL UNIQUE, "
            "role VARCHAR(25) NOT NULL, "
            "password_hash VARCHAR(256) NOT NULL, "
            "loyalty_level INT DEFAULT 0"
            ")"
        );
        
        db.executeNonQuery(
            "CREATE TABLE IF NOT EXISTS products ("
            "product_id SERIAL PRIMARY KEY, "
            "name VARCHAR(100) NOT NULL, "
            "price DECIMAL(10,2), "
            "stock_quantity INT"
            ")"
        );
        
        db.executeNonQuery(
            "CREATE TABLE IF NOT EXISTS orders ("
            "order_id SERIAL PRIMARY KEY, "
            "user_id INT, "
            "status VARCHAR(50), "
            "total_price DECIMAL(10, 2) default 0, "
            "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ")"
        );
        
        db.executeNonQuery(
            "CREATE TABLE IF NOT EXISTS order_items ("
            "order_item_id SERIAL PRIMARY KEY, "
            "order_id INT, "
            "product_id INT, "
            "quantity INT, "
            "price DECIMAL(10, 2)"
            ")"
        );
        
        db.executeNonQuery(
            "CREATE TABLE IF NOT EXISTS order_status_history ("
            "history_id SERIAL PRIMARY KEY, "
            "order_id INT, "
            "old_status VARCHAR(100), "
            "new_status VARCHAR(100), "
            "changed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "changed_by INT"
            ")"
        );
        
        db.executeNonQuery(
            "CREATE TABLE IF NOT EXISTS audit_log ("
            "log_id SERIAL PRIMARY KEY, "
            "entity_type VARCHAR(20), "
            "entity_id INT, "
            "operation VARCHAR(20), "
            "performed_by INT, "
            "performed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "details TEXT"
            ")"
        );
        
        std::cout << "All tables created\n";
        
        db.executeNonQuery(
            "INSERT INTO users (user_id, name, email, role, password_hash, loyalty_level) VALUES "
            "(1, 'Admin User', 'admin@store.com', 'admin', 'admin123', 1), "
            "(2, 'Manager User', 'manager@store.com', 'manager', 'manager123', 0), "
            "(3, 'Customer User', 'customer@store.com', 'customer', 'customer123', 0) "
            "ON CONFLICT (user_id) DO NOTHING"
        );
        std::cout << "Users added\n";
        
        db.executeNonQuery(
            "INSERT INTO products (name, price, stock_quantity) VALUES "
            "('Ноутбук', 50000.00, 10), "
            "('Смартфон', 30000.00, 20), "
            "('Наушники', 5000.00, 50) "
            "ON CONFLICT DO NOTHING"
        );
        std::cout << "Test products added\n";
        
        std::cout << "\nDatabase created fully!\n";
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error with creating tables: " << e.what() << "\n";
        return false;
    }
}