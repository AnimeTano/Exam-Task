#include "ManagerMenu.h"
#include <iostream>
#include <iomanip>
#include <algorithm>


namespace ManagerMenu {
    void handleManagerMenu(std::shared_ptr<Manager> manager, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger) {
        while (true) {
            UI::printSection("Manager Panel");
            std::cout << "1. View pending orders for approval\n";
            std::cout << "2. Approve order\n";
            std::cout << "3. Update product stock\n";
            std::cout << "4. View order details\n";
            std::cout << "5. Change order status\n";
            std::cout << "6. View approved orders history\n";
            std::cout << "7. View order status history\n";
            std::cout << "8. Show statistics\n";
            std::cout << "9. Exit\n";
            
            int choice = UI::getInput<int>("Select option (1-9): ", 1, 9);
            
            switch (choice) {
                case 1: handlePendingOrders(db); break;
                case 2: handleApproveOrder(db, logger); break;
                case 3: handleUpdateStock(db, logger); break;
                case 4: handleViewOrderDetails(db); break;
                case 5: handleChangeOrderStatus(db, logger); break;
                case 6: handleApprovedOrdersHistory(db); break;
                case 7: handleOrderStatusHistory(db, logger); break;
                case 8: showManagerStatistics(db); break;
                case 9: return;
            }
            
            UI::waitForEnter();
        }
    }
    
    void handlePendingOrders(std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("Pending Orders for Approval");
        
        try {
            auto pending_orders = db->executeQuery(
                "SELECT o.order_id, u.name, o.total_price, o.order_date, "
                "COUNT(oi.order_item_id) as items_count "
                "FROM orders o "
                "JOIN users u ON o.user_id = u.user_id "
                "LEFT JOIN order_items oi ON o.order_id = oi.order_id "
                "WHERE o.status = 'pending' "
                "GROUP BY o.order_id, u.name, o.total_price, o.order_date "
                "ORDER BY o.order_date"
            );
            
            if (pending_orders.empty()) {
                UI::printInfo("No pending orders for approval.");
                return;
            }
            
            std::cout << "Found " << pending_orders.size() << " pending orders:\n\n";
            
            for (const auto& order : pending_orders) {
                std::cout << "Order #" << order[0]
                          << " | Customer: " << order[1]
                          << " | Amount: " << order[2] << " RUB"
                          << " | Date: " << order[3]
                          << " | Items: " << order[4] << "\n";
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error fetching pending orders: " + std::string(e.what()));
        }
    }
    
    void handleApproveOrder(std::shared_ptr<DatabaseConnection<>> db,
                           std::shared_ptr<Logger> logger) {
        UI::printSection("Approve Order");
        
        int order_id = UI::getInput<int>("Enter order ID to approve: ", 1);
        
        try {
            auto order_check = db->executeQuery(
                "SELECT status FROM orders WHERE order_id = " + std::to_string(order_id)
            );
            
            if (order_check.empty()) {
                UI::printError("Order #" + std::to_string(order_id) + " not found!");
                return;
            }
            
            std::string current_status = order_check[0][0];
            
            if (current_status != "pending") {
                UI::printError("Order #" + std::to_string(order_id) + 
                                     " cannot be approved. Current status: " + current_status);
                return;
            }
            
            std::string new_status = "completed";
            db->executeNonQuery(
                "UPDATE orders SET status = '" + new_status + 
                "' WHERE order_id = " + std::to_string(order_id)
            );
            
            logger->logOrderStatusChange(order_id, current_status, new_status);
            logger->logAudit("order", order_id, "update", 
                           "Order approved by manager");
            
            UI::printSuccess("Order #" + std::to_string(order_id) + " approved successfully!");
            
        } catch (const std::exception& e) {
            UI::printError("Error approving order: " + std::string(e.what()));
        }
    }
    
    void handleUpdateStock(std::shared_ptr<DatabaseConnection<>> db,
                          std::shared_ptr<Logger> logger) {
        UI::printSection("Update Product Stock");
        
        int product_id = UI::getInput<int>("Enter product ID: ", 1);
        
        try {
            auto product_info = db->executeQuery(
                "SELECT name, stock_quantity FROM products WHERE product_id = " + 
                std::to_string(product_id)
            );
            
            if (product_info.empty()) {
                UI::printError("Product #" + std::to_string(product_id) + " not found!");
                return;
            }
            
            std::string product_name = product_info[0][0];
            int current_stock = std::stoi(product_info[0][1]);
            
            std::cout << "Product: " << product_name << "\n";
            std::cout << "Current stock: " << current_stock << "\n\n";
            
            int new_quantity = UI::getInput<int>("Enter new stock quantity: ", 0);
            
            db->executeNonQuery(
                "UPDATE products SET stock_quantity = " + std::to_string(new_quantity) + 
                " WHERE product_id = " + std::to_string(product_id)
            );
            
            logger->logAudit("product", product_id, "update", 
                           "Stock updated by manager: " + std::to_string(new_quantity));
            
            UI::printSuccess("Stock updated successfully!");
            
        } catch (const std::exception& e) {
            UI::printError("Error updating stock: " + std::string(e.what()));
        }
    }
    
    void handleChangeOrderStatus(std::shared_ptr<DatabaseConnection<>> db,
                                std::shared_ptr<Logger> logger) {
        UI::printSection("Change Order Status");
        
        int order_id = UI::getInput<int>("Enter order ID: ", 1);
        
        try {
            auto order_check = db->executeQuery(
                "SELECT status FROM orders WHERE order_id = " + std::to_string(order_id)
            );
            
            if (order_check.empty()) {
                UI::printError("Order not found!");
                return;
            }
            
            std::string current_status = order_check[0][0];
            std::cout << "Current status: " << current_status << "\n\n";
            
            std::cout << "Manager can change status to:\n";
            std::cout << "1. completed\n";
            std::cout << "2. canceled\n";
            std::cout << "Enter new status: ";
            
            std::string new_status;
            std::cin >> new_status;
            UI::clearInputBuffer();
            
            if (!canManagerChangeStatus(current_status, new_status)) {
                UI::printError("Manager cannot change status from '" + 
                                     current_status + "' to '" + new_status + "'");
                return;
            }
            
            db->executeNonQuery(
                "UPDATE orders SET status = '" + new_status + 
                "' WHERE order_id = " + std::to_string(order_id)
            );
            
            logger->logOrderStatusChange(order_id, current_status, new_status);
            logger->logAudit("order", order_id, "update", 
                           "Status changed by manager");
            
            UI::printSuccess("Order status updated successfully!");
            
        } catch (const std::exception& e) {
            UI::printError("Error changing order status: " + std::string(e.what()));
        }
    }
    
    bool canManagerChangeStatus(const std::string& current_status, 
                                const std::string& new_status) {
        if (current_status != "pending") {
            return false;
        }
        
        return (new_status == "completed" || new_status == "canceled");
    }
    
    void showManagerStatistics(std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("Manager Statistics");
        
        try {
            auto product_stats = db->executeQuery(
                "SELECT COUNT(*) as total_products, "
                "SUM(stock_quantity) as total_stock, "
                "SUM(CASE WHEN stock_quantity = 0 THEN 1 ELSE 0 END) as out_of_stock "
                "FROM products"
            );
            
            if (!product_stats.empty()) {
                std::cout << " fProduct Statistics:\n";
                std::cout << "   Total products: " << product_stats[0][0] << "\n";
                std::cout << "   Total items in stock: " << product_stats[0][1] << "\n";
                std::cout << "   Out of stock products: " << product_stats[0][2] << "\n";
            }
            
            auto order_stats = db->executeQuery(
                "SELECT COUNT(*) as total_orders, "
                "SUM(CASE WHEN status = 'pending' THEN 1 ELSE 0 END) as pending, "
                "SUM(CASE WHEN status = 'completed' THEN 1 ELSE 0 END) as completed, "
                "SUM(CASE WHEN status = 'canceled' THEN 1 ELSE 0 END) as canceled, "
                "SUM(CASE WHEN status = 'returned' THEN 1 ELSE 0 END) as returned "
                "FROM orders"
            );
            
            if (!order_stats.empty()) {
                std::cout << "\n Order Statistics:\n";
                std::cout << "   Total orders: " << order_stats[0][0] << "\n";
                std::cout << "   Pending: " << order_stats[0][1] << "\n";
                std::cout << "   Completed: " << order_stats[0][2] << "\n";
                std::cout << "   Canceled: " << order_stats[0][3] << "\n";
                std::cout << "   Returned: " << order_stats[0][4] << "\n";
            }
            
            auto daily_stats = db->executeQuery(
                "SELECT DATE(order_date) as order_day, "
                "COUNT(*) as orders_count, "
                "SUM(total_price) as daily_revenue "
                "FROM orders "
                "WHERE order_date >= NOW() - INTERVAL '7 days' "
                "GROUP BY DATE(order_date) "
                "ORDER BY order_day DESC"
            );
            
            if (!daily_stats.empty()) {
                std::cout << "\nLast 7 Days Statistics:\n";
                for (const auto& day : daily_stats) {
                    std::cout << "   " << day[0] << ": " << day[1] 
                              << " orders, " << day[2] << " RUB revenue\n";
                }
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error fetching statistics: " + std::string(e.what()));
        }
    }
    
    void handleViewOrderDetails(std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("Order Details");
        
        int order_id = UI::getInput<int>("Enter order ID: ", 1);
        
        try {
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
        }
    }

    void handleApprovedOrdersHistory(std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("Approved Orders History");
        
        try {
            auto approved_orders = db->executeQuery(
                "SELECT o.order_id, u.name, o.total_price, o.order_date, o.status "
                "FROM orders o JOIN users u ON o.user_id = u.user_id "
                "WHERE o.status = 'completed' OR o.status = 'canceled' "
                "ORDER BY o.order_date DESC LIMIT 20"
            );
            
            if (approved_orders.empty()) {
                UI::printInfo("No approved orders found.");
                return;
            }
            
            std::cout << "Recently approved/canceled orders:\n";
            for (const auto& order : approved_orders) {
                std::cout << "Order #" << order[0]
                        << " | Customer: " << order[1]
                        << " | Amount: " << order[2] << " RUB"
                        << " | Date: " << order[3]
                        << " | Status: " << order[4] << "\n";
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
        }
    }

    void handleOrderStatusHistory(std::shared_ptr<DatabaseConnection<>> db,
                                            std::shared_ptr<Logger> logger) {
        UI::printSection("Order Status History");
        
        int order_id = UI::getInput<int>("Enter order ID: ", 1);
        
        try {
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