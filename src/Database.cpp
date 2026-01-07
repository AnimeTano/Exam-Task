#include "include/Database.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>

// Явные инстанциации для поддерживаемых типов
template class DatabaseConnection<std::string>;
template class DatabaseConnection<const char*>;

// Фабричный метод для создания подключения
std::unique_ptr<DatabaseConnection<>> createDatabaseConnection(
    const std::string& host,
    const std::string& port,
    const std::string& dbname,
    const std::string& user,
    const std::string& password
) {
    std::string connection_string = 
        "host=" + host + " " +
        "port=" + port + " " +
        "dbname=" + dbname + " " +
        "user=" + user + " " +
        "password=" + password;
    
    return std::make_unique<DatabaseConnection<>>(connection_string);
}

// Реализация TransactionGuard
TransactionGuard::TransactionGuard(DatabaseConnection<>& db_ref) 
    : db(db_ref), committed(false) {
    db.beginTransaction();
}

TransactionGuard::~TransactionGuard() {
    if (!committed) {
        try {
            db.rollbackTransaction();
        } catch (...) {
            // Игнорируем ошибки при откате в деструкторе
        }
    }
}

void TransactionGuard::commit() {
    db.commitTransaction();
    committed = true;
}

// Инициализация базы данных (создание таблиц)
void initializeDatabase(DatabaseConnection<>& db) {
    try {
        // Создание таблиц в правильном порядке (сначала таблицы без внешних ключей)
        std::vector<std::string> init_queries = {
            // Таблица пользователей
            R"(
            CREATE TABLE IF NOT EXISTS users (
                user_id SERIAL PRIMARY KEY,
                name VARCHAR(100) NOT NULL,
                email VARCHAR(100) UNIQUE NOT NULL,
                role VARCHAR(25) CHECK (role IN ('admin', 'manager', 'customer')) NOT NULL,
                password_hash VARCHAR(200) NOT NULL,
                loyalty_level INTEGER DEFAULT 0 CHECK (loyalty_level IN (0, 1))
            );
            )",

            // Таблица продуктов
            R"(
            CREATE TABLE IF NOT EXISTS products (
                product_id SERIAL PRIMARY KEY,
                name VARCHAR(100) NOT NULL,
                price DECIMAL(10,2) CHECK (price > 0) NOT NULL,
                stock_quantity INTEGER CHECK (stock_quantity >= 0) DEFAULT 0
            );
            )",

            // Таблица заказов (зависит от users)
            R"(
            CREATE TABLE IF NOT EXISTS orders (
                order_id SERIAL PRIMARY KEY,
                user_id INTEGER REFERENCES users(user_id) ON DELETE CASCADE,
                status VARCHAR(50) CHECK (status IN ('pending', 'completed', 'canceled', 'returned')) DEFAULT 'pending',
                total_price DECIMAL(10, 2) DEFAULT 0,
                order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            )",

            // Таблица элементов заказа (зависит от orders и products)
            R"(
            CREATE TABLE IF NOT EXISTS order_items (
                order_item_id SERIAL PRIMARY KEY,
                order_id INTEGER REFERENCES orders(order_id) ON DELETE CASCADE,
                product_id INTEGER REFERENCES products(product_id) ON DELETE CASCADE,
                quantity INTEGER CHECK (quantity > 0) NOT NULL,
                price DECIMAL(10, 2) CHECK (price >= 0) NOT NULL
            );
            )",

            // Таблица истории статусов заказов (зависит от orders и users)
            R"(
            CREATE TABLE IF NOT EXISTS order_status_history (
                history_id SERIAL PRIMARY KEY,
                order_id INTEGER REFERENCES orders(order_id) ON DELETE CASCADE,
                old_status VARCHAR(50),
                new_status VARCHAR(50) NOT NULL,
                changed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                changed_by INTEGER REFERENCES users(user_id) ON DELETE SET NULL
            );
            )",

            // Таблица аудита (зависит от users)
            R"(
            CREATE TABLE IF NOT EXISTS audit_log (
                log_id SERIAL PRIMARY KEY,
                entity_type VARCHAR(20) CHECK (entity_type IN ('order', 'product', 'user')) NOT NULL,
                entity_id INTEGER NOT NULL,
                operation VARCHAR(20) CHECK (operation IN ('insert', 'update', 'delete')) NOT NULL,
                performed_by INTEGER REFERENCES users(user_id) ON DELETE SET NULL,
                performed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                details TEXT
            );
            )"
        };
        
        // Выполняем все запросы в одной транзакции
        TransactionGuard guard(db);
        for (const auto& query : init_queries) {
            db.executeNonQuery(query);
        }
        guard.commit();
        
        std::cout << "Database initialized successfully." << std::endl;
        
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Database initialization failed: ") + e.what());
    }
}

// Вспомогательные функции для работы с базой данных
namespace DatabaseUtils {
    
    // Проверка существования таблицы
    bool tableExists(DatabaseConnection<>& db, const std::string& table_name) {
        try {
            std::string query = 
                "SELECT EXISTS ("
                "   SELECT FROM information_schema.tables "
                "   WHERE table_schema = 'public' "
                "   AND table_name = $1"
                ");";
            
            auto result = db.executePreparedQuery("check_table_exists", query, table_name);
            
            if (!result.empty() && !result[0].empty()) {
                return result[0][0] == "t";
            }
            return false;
            
        } catch (const std::exception& e) {
            std::cerr << "Error checking table existence: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Очистка всех данных (только для тестов)
    void clearAllData(DatabaseConnection<>& db) {
        try {
            TransactionGuard guard(db);
            
            // Удаление данных в правильном порядке (из-за внешних ключей)
            db.executeNonQuery("DELETE FROM audit_log;");
            db.executeNonQuery("DELETE FROM order_status_history;");
            db.executeNonQuery("DELETE FROM order_items;");
            db.executeNonQuery("DELETE FROM orders;");
            db.executeNonQuery("DELETE FROM products;");
            db.executeNonQuery("DELETE FROM users;");
            
            // Сброс последовательностей
            db.executeNonQuery("ALTER SEQUENCE users_user_id_seq RESTART WITH 1;");
            db.executeNonQuery("ALTER SEQUENCE products_product_id_seq RESTART WITH 1;");
            db.executeNonQuery("ALTER SEQUENCE orders_order_id_seq RESTART WITH 1;");
            db.executeNonQuery("ALTER SEQUENCE order_items_order_item_id_seq RESTART WITH 1;");
            db.executeNonQuery("ALTER SEQUENCE order_status_history_history_id_seq RESTART WITH 1;");
            db.executeNonQuery("ALTER SEQUENCE audit_log_log_id_seq RESTART WITH 1;");
            
            guard.commit();
            
            std::cout << "All data cleared successfully" << std::endl;
            
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to clear data: ") + e.what());
        }
    }
    
    // Получить статистику базы данных
    std::string getDatabaseStats(DatabaseConnection<>& db) {
        try {
            std::string stats;
            
            auto tables = db.executeQuery(
                "SELECT table_name FROM information_schema.tables "
                "WHERE table_schema = 'public' ORDER BY table_name;"
            );
            
            stats += "Database Statistics:\n";
            stats += "====================\n";
            
            for (const auto& table_row : tables) {
                if (!table_row.empty()) {
                    std::string table_name = table_row[0];
                    auto count_result = db.executeQuery(
                        "SELECT COUNT(*) FROM " + table_name + ";"
                    );
                    
                    if (!count_result.empty() && !count_result[0].empty()) {
                        stats += table_name + ": " + count_result[0][0] + " records\n";
                    }
                }
            }
            
            return stats;
            
        } catch (const std::exception& e) {
            return std::string("Error getting stats: ") + e.what();
        }
    }
    
    // Создание тестовых данных
    void createSampleData(DatabaseConnection<>& db) {
        try {
            TransactionGuard guard(db);
            
            // Создание тестового администратора
            db.executeNonQuery(
                "INSERT INTO users (name, email, role, password_hash, loyalty_level) "
                "VALUES ('Admin User', 'admin@store.com', 'admin', 'admin_hash', 1) "
                "ON CONFLICT (email) DO NOTHING;"
            );
            
            // Создание тестового менеджера
            db.executeNonQuery(
                "INSERT INTO users (name, email, role, password_hash, loyalty_level) "
                "VALUES ('Manager User', 'manager@store.com', 'manager', 'manager_hash', 0) "
                "ON CONFLICT (email) DO NOTHING;"
            );
            
            // Создание тестового покупателя
            db.executeNonQuery(
                "INSERT INTO users (name, email, role, password_hash, loyalty_level) "
                "VALUES ('Customer User', 'customer@store.com', 'customer', 'customer_hash', 0) "
                "ON CONFLICT (email) DO NOTHING;"
            );
            
            // Создание тестовых продуктов
            std::vector<std::tuple<std::string, double, int>> products = {
                {"Laptop", 999.99, 10},
                {"Mouse", 29.99, 50},
                {"Keyboard", 79.99, 30},
                {"Monitor", 249.99, 15},
                {"Headphones", 149.99, 25}
            };
            
            for (const auto& [name, price, quantity] : products) {
                db.executeNonQuery(
                    "INSERT INTO products (name, price, stock_quantity) "
                    "VALUES ('" + name + "', " + std::to_string(price) + 
                    ", " + std::to_string(quantity) + ") "
                    "ON CONFLICT DO NOTHING;"
                );
            }
            
            guard.commit();
            
            std::cout << "Sample data created successfully." << std::endl;
            
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to create sample data: ") + e.what());
        }
    }
}