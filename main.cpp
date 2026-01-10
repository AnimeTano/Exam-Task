#include "include/Database.h"
#include "include/User.h"
#include "include/Order.h"
#include "include/Payment.h"
#include "include/SqlScript.h"
#include "include/AuditManager.h"
#include "include/ConsoleUtils.h"
#include <iostream>
#include <memory>
#include <algorithm>
#include <windows.h> 


auto checkPermission = [](const std::string& user_role, const std::string& required_role) -> bool {
    if (required_role == "admin") return user_role == "admin";
    if (required_role == "manager") return user_role == "admin" || user_role == "manager";
    return true;
};


void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


auto getPassword(){
    std::ifstream file("bin/config.txt");
    std::string line;

    if (file.fail()){
        std::cout << "Error with read password from file" << "\n";
        return line;
    }

    file >> line;

    file.close();
    return line;
}


int main() {
    ConsoleUtils::setupConsole();
    std::string password = getPassword();

    std::cout << "=== Store System ===\n";
    
    #ifdef _WIN32
        Sleep(1000); 
    #endif
    
    try {
        std::string connection_string = "host=localhost dbname=store user=postgres password=" + password;
        
        auto db = std::make_shared<DatabaseConnection<>>(connection_string);
        
        if (!db->isConnected()) {
            std::cerr << "Error with connect to db" << "\n";
            
            #ifdef _WIN32
                std::cout << "\nEnter to exit";
                clearInputBuffer();
                std::cin.get();
            #endif
            
            return 1;
        }

        auto logger = std::make_shared<Logger>(db);
        AuditManager::initialize(db);
        
        try {
            if (!SqlScript::initializeDatabase(*db, "sql")) {
                std::cout << "Db isn`t initialize fully\n";
            }

            logger->setUser(1);
            logger->logAudit("user", 1, "update", "System started - Admin logged in");
            
        } catch (const std::exception& e) {
            std::cerr << "Warning: " << e.what() << "\n";
        }
        
        while (true) {
            std::cout << "\n=== Choose your role ===\n";
            std::cout << "1. Admin\n";
            std::cout << "2. Manager\n";
            std::cout << "3. Customer\n";
            std::cout << "4. Exit\n";
            std::cout << "Enter your choose (1-4): ";
            
            int choice = 0;
            if (!(std::cin >> choice)) {
                std::cout << "Invalid choice!\n";
                clearInputBuffer();
                continue;
            }
            
            if (choice == 4) {
                std::cout << "\nThe end!\n";
                #ifdef _WIN32
                    Sleep(1000);
                #endif

                break;
            }
            
            std::unique_ptr<User> user;
            
            switch(choice) {
                case 1: {
                    user = std::make_unique<Admin>(1, "Админ", "admin@store.com", "pass", true);
                    logger->setUser(1);

                    std::cout << "\n=== Admin panel ===\n";
                    std::cout << "1. Add product\n";
                    std::cout << "2. Show all orders\n";
                    std::cout << "3. Show audit logs\n";
                    std::cout << "4. Create CSV-report \n";
                    std::cout << "5. Back to main menu\n";
                    std::cout << "Choice: ";
                    
                    int admin_choice = 0;
                    std::cin >> admin_choice;
                    
                    if (admin_choice == 1) {
                        std::string name = "";
                        double price = 0.0;
                        int quantity = 0;
                        
                        std::cout << "Product name: ";
                        clearInputBuffer();
                        std::getline(std::cin, name);

                        std::cout << "Price: ";
                        std::cin >> price;

                        std::cout << "Quantity: ";
                        std::cin >> quantity;
                        
                        logger->logAudit("product", quantity, "insert", 
                                        "added product: " + name + " with price " + std::to_string(price));
                        
                        try {
                            std::string query = "INSERT INTO products (name, price, stock_quantity) VALUES ('" +
                                               name + "', " + std::to_string(price) + ", " + 
                                               std::to_string(quantity) + ")";
                            db->executeNonQuery(query);
                            std::cout << "Product added!\n";
                        } catch (const std::exception& e) {
                            std::cerr << "Error: " << e.what() << "\n";
                        }
                    } else if (admin_choice == 2) {
                        try {
                            auto orders = db->executeQuery("SELECT * FROM orders");

                            if (orders.empty()) {
                                std::cout << "No orders\n";
                            } else {
                                std::cout << "Orders list:\n";
                                for (const auto& order_row : orders) {
                                    for (const auto& field : order_row) {
                                        std::cout << field << " | ";
                                    }
                                    std::cout << "\n";
                                }
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "Error: " << e.what() << "\n";
                        }
                    } else if (admin_choice == 3) {
                        std::cout << "\n=== Audit logs ===\n";
                        auto logs = logger->getAllAuditLogs();

                        if (logs.empty()) {
                            std::cout << "No logs\n";
                        } else {
                            for (const auto& log : logs) {
                                for (const auto& field : log) { 
                                    std::cout << field << " | ";
                                }
                                std::cout << "\n";
                            }
                        }
                    } else if (admin_choice == 4) {
                        std::cout << "\nCreate report\n";
                        if (logger->generateCSVReport("reports/audit_report.csv")) {
                            std::cout << "Report created succesfully!\n";
                        }
                    }
                    break;
                }
                    
                case 2: {
                    user = std::make_unique<Manager>(2, "Менеджер", "manager@store.com", "pass", false);
                    logger->setUser(2);
                    
                    std::cout << "\n=== Manager panel ===\n";
                    
                    try {
                        auto products = db->executeQuery("SELECT COUNT(*) FROM products");

                        if (!products.empty()) {
                            std::cout << "Products in store: " << products[0][0] << "\n";
                        }
                    } catch (...) {
                        std::cout << "Products in store: 0\n";
                    }
                    
                    logger->logAudit("order", 0, "update", "Менеджер утвердил заказ");

                    #ifdef _WIN32
                        Sleep(1000);
                    #endif
                    
                    break;
                }
                    
                case 3: {
                    user = std::make_unique<Customer>(3, "Покупатель", "customer@store.com", "pass", false);
                    logger->setUser(3);
                    
                    std::cout << "\n=== Customer panel ===\n";
                    
                    try {
                        auto order = std::make_shared<Order>(1, 3);
                        order->addItem(OrderItem(1, 2, 321.0));
                        order->calculateTotal();

                        std::cout << "Order created! Summa: " << order->getTotalPrice() << " rub\n";

                        std::cout << "\n=== Payment Strategies ===\n";
                        std::cout << "1. Bank card\n";
                        std::cout << "2. E-wallet\n";
                        std::cout << "3. SBP\n";
                        std::cout << "Your choice (1-3): ";

                        int paymentChoice = 0;
                        std::cin >> paymentChoice;

                        std::unique_ptr<PaymentStrategy> strategy;
                        switch(paymentChoice) {
                            case 1:
                                strategy = std::make_unique<CreditCardPayment>();
                                break;
                            case 2:
                                strategy = std::make_unique<EWalletPayment>();
                                break;
                            case 3:
                                strategy = std::make_unique<SBPPayment>();
                                break;
                            default:
                                std::cout << "Invalid choice\n";
                                strategy = std::make_unique<CreditCardPayment>();
                        }

                        auto payment = std::make_unique<Payment>(order->getTotalPrice(), std::move(strategy));
                        
                        std::string strategyName = payment->getStrategyName();

                        if (payment->process()) {
                            order->setPayment(std::move(payment));
                            std::cout << "\nOrder paid!\n";

                            logger->logAudit("order", order->getOrderId(), "insert", 
                                           "created new order " + std::to_string(order->getTotalPrice()));
                            logger->logAudit("order", order->getOrderId(), "update", 
                                           "Order paid " + strategyName);
                            logger->logOrderStatusChange(order->getOrderId(), "pending", "completed");

                            std::string query = "INSERT INTO orders (user_id, status, total_price) VALUES (3, 'completed', " + 
                                               std::to_string(order->getTotalPrice()) + ")";
                            db->executeNonQuery(query);
                            
                            std::cout << "Order saved to db\n";
                        } else {
                            std::cerr << "\nError with paing\n";
                        }
                        
                    } catch (const std::exception& e) {
                        std::cerr << "Error: " << e.what() << "\n";
                    }
                    
                    #ifdef _WIN32
                        Sleep(1000);
                    #endif

                    break;
                }
                    
                default:
                    std::cout << "Invalid choice!\n";
                    continue;
            }
            
            if (user) {
                std::cout << "\nInformation about customer: ";
                user->showInfo();
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        
        #ifdef _WIN32
            std::cout << "\nEnter to exit \n";
            clearInputBuffer();
            std::cin.get();
        #endif
        
        return 1;
    }
    
    return 0;
}