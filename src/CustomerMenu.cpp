#include "CustomerMenu.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>


namespace CustomerMenu {
    void handleCustomerMenu(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger) {
        while (true) {
            UI::printSection("Customer Panel");
            std::cout << "1. Create new order\n";
            std::cout << "2. Add product to order\n";
            std::cout << "3. Remove product from order\n";
            std::cout << "4. View my orders\n";
            std::cout << "5. View order status\n";
            std::cout << "6. Make payment\n";
            std::cout << "7. Return order\n";
            std::cout << "8. View order status history\n";
            std::cout << "9. View available products\n";
            std::cout << "10. Exit\n";
            
            int choice = UI::getInput<int>("Select option (1-10): ", 1, 10);
            
            switch (choice) {
                case 1: handleCreateOrder(customer, db, logger); break;
                case 2: handleAddToOrder(customer, db); break;
                case 3: handleRemoveFromOrder(customer, db); break;
                case 4: handleViewMyOrders(customer, db); break;
                case 5: handleViewOrderStatus(customer, db); break;
                case 6: handleMakePayment(customer, db, logger); break;
                case 7: handleReturnOrder(customer, db, logger); break;
                case 8: handleOrderStatusHistory(customer, db, logger); break;
                case 9: showAvailableProducts(db); break;
                case 10: return;
            }
            
            UI::waitForEnter();
        }
    }
    
    void handleCreateOrder(std::shared_ptr<Customer> customer,
                          std::shared_ptr<DatabaseConnection<>> db,
                          std::shared_ptr<Logger> logger) {
        UI::printSection("Create New Order");
        
        try {
            auto new_order = customer->createOrder();
            
            if (!new_order) {
                UI::printError("Failed to create order!");
                return;
            }
            
            std::cout << "Order #" << new_order->getOrderId() << " created.\n\n";
            showAvailableProducts(db);
            
            while (true) {
                std::cout << "\nEnter product ID to add (0 to finish): ";
                int product_id;
                if (!(std::cin >> product_id)) {
                    UI::clearInputBuffer();
                    UI::printError("Invalid input!");
                    continue;
                }
                
                if (product_id == 0) {
                    break;
                }
                
                auto product_check = db->executeQuery(
                    "SELECT name, price, stock_quantity FROM products WHERE product_id = " + 
                    std::to_string(product_id)
                );
                
                if (product_check.empty()) {
                    UI::printError("Product not found!");
                    continue;
                }
                
                std::string product_name = product_check[0][0];
                double price = std::stod(product_check[0][1]);
                int available = std::stoi(product_check[0][2]);
                
                std::cout << "Product: " << product_name 
                          << ", Price: " << price << " RUB"
                          << ", Available: " << available << " pcs\n";
                
                int quantity = UI::getInput<int>("Enter quantity: ", 1, available);
                
                customer->addToOrder(new_order->getOrderId(), product_id, quantity);
                
                logger->logAudit("order_item", product_id, "insert",
                               "Added to order #" + std::to_string(new_order->getOrderId()));
                
                UI::printSuccess("Product added to order!");
            }
            
            std::string query = "INSERT INTO orders (user_id, status, total_price) VALUES (" +
                               std::to_string(customer->getUserId()) + ", 'pending', " +
                               std::to_string(new_order->getTotalPrice()) + ") RETURNING order_id";
            
            auto result = db->executeQuery(query);
            
            if (!result.empty()) {
                int db_order_id = std::stoi(result[0][0]);
                logger->logAudit("order", db_order_id, "insert",
                               "New order created by customer");
                
                UI::printSuccess("Order saved successfully!");
                std::cout << "Total amount: " << new_order->getTotalPrice() << " RUB\n";
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error creating order: " + std::string(e.what()));
        }
    }
    
    void handleMakePayment(std::shared_ptr<Customer> customer,
                          std::shared_ptr<DatabaseConnection<>> db,
                          std::shared_ptr<Logger> logger) {
        UI::printSection("Make Payment");
        
        int order_id = UI::getInput<int>("Enter order ID for payment: ", 1);
        
        try {
            auto order_check = db->executeQuery(
                "SELECT total_price, status FROM orders WHERE order_id = " + 
                std::to_string(order_id) + " AND user_id = " + 
                std::to_string(customer->getUserId())
            );
            
            if (order_check.empty()) {
                UI::printError("Order not found or access denied!");
                return;
            }
            
            double amount = std::stod(order_check[0][0]);
            std::string status = order_check[0][1];
            
            if (status != "pending") {
                UI::printError("Order cannot be paid. Current status: " + status);
                return;
            }
            
            if (amount <= 0) {
                UI::printError("Order amount must be greater than 0!");
                return;
            }
            
            UI::printSection("Select Payment Method");
            std::cout << "1. Credit Card\n";
            std::cout << "2. E-Wallet\n";
            std::cout << "3. SBP (Fast Payment System)\n";
            
            int method_choice = UI::getInput<int>("Select method (1-3): ", 1, 3);
            
            std::unique_ptr<PaymentStrategy> strategy;
            std::string method_name;
            
            switch (method_choice) {
                case 1:
                    strategy = std::make_unique<CreditCardPayment>();
                    method_name = "Credit Card";
                    break;
                case 2:
                    strategy = std::make_unique<EWalletPayment>();
                    method_name = "E-Wallet";
                    break;
                case 3:
                    strategy = std::make_unique<SBPPayment>();
                    method_name = "SBP";
                    break;
            }
            
            auto payment = std::make_unique<Payment>(amount, std::move(strategy));
            
            std::cout << "\nProcessing payment for order #" << order_id << "...\n";
            std::cout << "Amount: " << amount << " RUB\n";
            std::cout << "Method: " << method_name << "\n";
            
            if (payment->process()) {
                db->executeNonQuery(
                    "UPDATE orders SET status = 'completed' WHERE order_id = " + 
                    std::to_string(order_id)
                );
                
                logger->logAudit("order", order_id, "update",
                               "Order paid with " + method_name);
                logger->logOrderStatusChange(order_id, "pending", "completed");
                
                UI::printSuccess("Payment successful! Order #" + 
                                       std::to_string(order_id) + " is now completed.");
            } else {
                UI::printError("Payment failed!");
            }
            
        } catch (const std::exception& e) {
            UI::printError("Payment error: " + std::string(e.what()));
        }
    }
    
    void handleReturnOrder(std::shared_ptr<Customer> customer,
                          std::shared_ptr<DatabaseConnection<>> db,
                          std::shared_ptr<Logger> logger) {
        UI::printSection("Return Order");
        
        int order_id = UI::getInput<int>("Enter order ID for return: ", 1);
        
        try {
            auto can_return_result = db->executeQuery(
                "SELECT canReturnOrder(" + std::to_string(order_id) + ")"
            );
            
            if (can_return_result.empty() || can_return_result[0][0] != "t") {
                UI::printError("This order cannot be returned.");
                std::cout << "Reasons: order not completed or more than 30 days have passed.\n";
                return;
            }
            
            std::cout << "Order #" << order_id << " can be returned.\n";
            
            std::string confirm = UI::getStringInput("Confirm return? (yes/no): ");
            
            if (confirm == "yes" || confirm == "y") {
                db->executeNonQuery(
                    "UPDATE orders SET status = 'returned' WHERE order_id = " + 
                    std::to_string(order_id) + " AND user_id = " + 
                    std::to_string(customer->getUserId())
                );
                
                logger->logAudit("order", order_id, "update",
                               "Order returned by customer");
                logger->logOrderStatusChange(order_id, "completed", "returned");
                
                UI::printSuccess("Return request submitted successfully!");
                std::cout << "Refund will be processed within 3-5 business days.\n";
            } else {
                UI::printInfo("Return cancelled.");
            }
            
        } catch (const std::exception& e) {
            UI::printError("Return error: " + std::string(e.what()));
        }
    }
    
    void showAvailableProducts(std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("Available Products");
        
        try {
            auto products = db->executeQuery(
                "SELECT product_id, name, price, stock_quantity FROM products "
                "WHERE stock_quantity > 0 ORDER BY name"
            );
            
            if (products.empty()) {
                UI::printInfo("No products available at the moment.");
                return;
            }
            
            std::cout << std::left << std::setw(8) << "ID"
                      << std::setw(30) << "Name"
                      << std::setw(15) << "Price (RUB)"
                      << std::setw(10) << "In Stock" << "\n";
            std::cout << std::string(63, '-') << "\n";
            
            for (const auto& product : products) {
                std::cout << std::left << std::setw(8) << product[0]
                          << std::setw(30) << (product[1].size() > 28 ? 
                                               product[1].substr(0, 27) + "..." : product[1])
                          << std::setw(15) << product[2]
                          << std::setw(10) << product[3] << "\n";
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error loading products: " + std::string(e.what()));
        }
    }
    
    void handleViewMyOrders(std::shared_ptr<Customer> customer,
                           std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("My Orders");
        
        try {
            auto my_orders = db->executeQuery(
                "SELECT order_id, status, total_price, order_date "
                "FROM orders WHERE user_id = " + std::to_string(customer->getUserId()) +
                " ORDER BY order_date DESC"
            );
            
            if (my_orders.empty()) {
                UI::printInfo("You have no orders yet.");
                return;
            }
            
            std::cout << std::left << std::setw(10) << "Order ID"
                      << std::setw(15) << "Status"
                      << std::setw(15) << "Amount (RUB)"
                      << std::setw(25) << "Date" << "\n";
            std::cout << std::string(65, '-') << "\n";
            
            double total_spent = 0.0;
            for (const auto& order : my_orders) {
                std::cout << std::left << std::setw(10) << order[0]
                          << std::setw(15) << order[1]
                          << std::setw(15) << order[2]
                          << std::setw(25) << order[3] << "\n";
                
                if (order[1] == "completed" || order[1] == "returned") {
                    total_spent += std::stod(order[2]);
                }
            }
            
            std::cout << "\nStatistics:\n";
            std::cout << "   Total orders: " << my_orders.size() << "\n";
            std::cout << "   Total spent: " << total_spent << " RUB\n";
            
        } catch (const std::exception& e) {
            UI::printError("Error loading orders: " + std::string(e.what()));
        }
    }

    void handleAddToOrder(std::shared_ptr<Customer> customer,
                                    std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("Add to Order");
        
        int order_id = UI::getInput<int>("Enter order ID: ", 1);
        int product_id = UI::getInput<int>("Enter product ID: ", 1);
        int quantity = UI::getInput<int>("Enter quantity: ", 1);
        
        try {
            auto order_check = db->executeQuery(
                "SELECT order_id FROM orders WHERE order_id = " + 
                std::to_string(order_id) + " AND user_id = " + 
                std::to_string(customer->getUserId())
            );
            
            if (order_check.empty()) {
                UI::printError("Order not found or access denied!");
                return;
            }
            
            auto product_check = db->executeQuery(
                "SELECT name, price, stock_quantity FROM products WHERE product_id = " + 
                std::to_string(product_id)
            );
            
            if (product_check.empty()) {
                UI::printError("Product not found!");
                return;
            }
            
            int available = std::stoi(product_check[0][2]);
            if (available < quantity) {
                UI::printError("Not enough stock! Available: " + std::to_string(available));
                return;
            }
            
            double price = std::stod(product_check[0][1]);
            
            std::string query = 
                "INSERT INTO order_items (order_id, product_id, quantity, price) "
                "VALUES (" + std::to_string(order_id) + ", " + std::to_string(product_id) + 
                ", " + std::to_string(quantity) + ", " + std::to_string(price) + ") "
                "ON CONFLICT (order_id, product_id) DO UPDATE SET quantity = order_items.quantity + " + 
                std::to_string(quantity);
            
            db->executeNonQuery(query);
            
            db->executeNonQuery(
                "UPDATE orders SET total_price = ("
                "SELECT COALESCE(SUM(quantity * price), 0) FROM order_items "
                "WHERE order_id = " + std::to_string(order_id) + ") "
                "WHERE order_id = " + std::to_string(order_id)
            );
            
            UI::printSuccess("Product added to order!");
            
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
        }
    }

    void handleRemoveFromOrder(std::shared_ptr<Customer> customer,
                                            std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("Remove from Order");
        
        int order_id = UI::getInput<int>("Enter order ID: ", 1);
        int product_id = UI::getInput<int>("Enter product ID: ", 1);
        
        try {
            auto order_check = db->executeQuery(
                "SELECT order_id FROM orders WHERE order_id = " + 
                std::to_string(order_id) + " AND user_id = " + 
                std::to_string(customer->getUserId())
            );
            
            if (order_check.empty()) {
                UI::printError("Order not found or access denied!");
                return;
            }
            
            std::string query = 
                "DELETE FROM order_items WHERE order_id = " + std::to_string(order_id) + 
                " AND product_id = " + std::to_string(product_id);
            
            db->executeNonQuery(query);
            
            db->executeNonQuery(
                "UPDATE orders SET total_price = ("
                "SELECT COALESCE(SUM(quantity * price), 0) FROM order_items "
                "WHERE order_id = " + std::to_string(order_id) + ") "
                "WHERE order_id = " + std::to_string(order_id)
            );
            
            UI::printSuccess("Product removed from order!");
            
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
        }
    }

    void handleViewOrderStatus(std::shared_ptr<Customer> customer,
                                            std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("View Order Status");
        
        int order_id = UI::getInput<int>("Enter order ID: ", 1);
        
        try {
            auto order_status = db->executeQuery(
                "SELECT status FROM orders WHERE order_id = " + 
                std::to_string(order_id) + " AND user_id = " + 
                std::to_string(customer->getUserId())
            );
            
            if (order_status.empty()) {
                UI::printError("Order not found or access denied!");
                return;
            }
            
            std::cout << "Order #" << order_id << " status: " << order_status[0][0] << "\n";
            
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
        }
    }

    void handleOrderStatusHistory(std::shared_ptr<Customer> customer,
                                            std::shared_ptr<DatabaseConnection<>> db,
                                            std::shared_ptr<Logger> logger) {
        UI::printSection("Order Status History");
        
        int order_id = UI::getInput<int>("Enter order ID: ", 1);
        
        try {
            auto order_check = db->executeQuery(
                "SELECT order_id FROM orders WHERE order_id = " + 
                std::to_string(order_id) + " AND user_id = " + 
                std::to_string(customer->getUserId())
            );
            
            if (order_check.empty()) {
                UI::printError("Order not found or access denied!");
                return;
            }
            
            auto history = logger->getOrderStatusHistory(order_id);
            
            if (history.empty()) {
                UI::printInfo("No history found for order #" + std::to_string(order_id));
                return;
            }
            
            std::cout << "\nStatus History for Order #" << order_id << ":\n";
            for (const auto& record : history) {
                if (record.size() >= 5) {
                    std::cout << record[4] << ": " << record[2] 
                            << " → " << record[3] << "\n";
                }
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
        }
    }
}