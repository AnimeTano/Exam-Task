#include "AdminMenu.h"
#include <iostream>
#include <iomanip>


namespace AdminMenu {
    void handleAdminMenu(std::shared_ptr<Admin> admin, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger) {
        while (true) {
            UI::printSection("Admin Panel");
            std::cout << "1. Add new product\n";
            std::cout << "2. Update product information\n";
            std::cout << "3. Delete product\n";
            std::cout << "4. View all orders\n";
            std::cout << "5. View order details\n";
            std::cout << "6. Change order status\n";
            std::cout << "7. View order status history\n";
            std::cout << "8. View audit logs\n";
            std::cout << "9. Generate report (CSV)\n";
            std::cout << "10. Exit\n";
            
            int choice = UI::getInput<int>("Select option (1-10): ", 1, 10);
            
            switch (choice) {
                case 1: handleAddProduct(db, logger); break;
                case 2: handleUpdateProduct(db, logger); break;
                case 3: handleDeleteProduct(db, logger); break;
                case 4: handleViewAllOrders(db); break;
                case 5: handleOrderDetails(db); break;
                case 6: handleChangeOrderStatus(db, logger); break;
                case 7: handleOrderStatusHistory(db, logger); break;
                case 8: handleAuditLogs(db, logger); break;
                case 9: handleGenerateReport(db, logger); break;
                case 10: return;
            }
            
            UI::waitForEnter();
        }
    }
    
    void handleAddProduct(std::shared_ptr<DatabaseConnection<>> db,
                         std::shared_ptr<Logger> logger) {
        UI::printSection("Add New Product");
        
        std::string name = UI::getStringInput("Product name: ");
        double price = UI::getDoubleInput("Product price: ", 0.01);
        int quantity = UI::getInput<int>("Stock quantity: ", 0);
        
        try {
            auto existing = db->executeQuery(
                "SELECT name FROM products WHERE LOWER(name) = LOWER('" + name + "')"
            );
            
            if (!existing.empty()) {
                UI::printError("Product with this name already exists!");
                return;
            }
            
            std::string query = "INSERT INTO products (name, price, stock_quantity) VALUES ('" +
                               name + "', " + std::to_string(price) + ", " +
                               std::to_string(quantity) + ")";
            db->executeNonQuery(query);
            
            logger->logAudit("product", 0, "insert", 
                           "Admin added product: " + name);
            UI::printSuccess("Product added successfully!");
            
        } catch (const std::exception& e) {
            UI::printError("Error adding product: " + std::string(e.what()));
        }
    }
    
    void handleViewAllOrders(std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("All Orders");
        
        try {
            auto orders = db->executeQuery(
                "SELECT o.order_id, u.name, o.status, o.total_price, "
                "o.order_date, COUNT(oi.order_item_id) as items_count "
                "FROM orders o "
                "LEFT JOIN users u ON o.user_id = u.user_id "
                "LEFT JOIN order_items oi ON o.order_id = oi.order_id "
                "GROUP BY o.order_id, u.name, o.status, o.total_price, o.order_date "
                "ORDER BY o.order_date DESC"
            );
            
            if (orders.empty()) {
                UI::printInfo("No orders in the system.");
                return;
            }
            
            double total_revenue = 0;
            for (const auto& order : orders) {
                total_revenue += std::stod(order[3]);
                
                std::cout << "Order #" << order[0] 
                          << " | Customer: " << order[1]
                          << " | Status: " << order[2]
                          << " | Amount: " << order[3] << " rub"
                          << " | Date: " << order[4]
                          << " | Items: " << order[5] << "\n";
            }
            
            std::cout << "\n--- Statistics ---\n";
            std::cout << "Total orders: " << orders.size() << "\n";
            std::cout << "Total revenue: " << total_revenue << " rub\n";
            
        } catch (const std::exception& e) {
            UI::printError("Error fetching orders: " + std::string(e.what()));
        }
    }

    void handleUpdateProduct(std::shared_ptr<DatabaseConnection<>> db,
                                    std::shared_ptr<Logger> logger) {
        UI::printSection("Update Product");
        
        int product_id = UI::getInput<int>("Enter product ID to update: ", 1);
        
        try {
            auto product_info = db->executeQuery(
                "SELECT name, price, stock_quantity FROM products WHERE product_id = " + 
                std::to_string(product_id)
            );
            
            if (product_info.empty()) {
                UI::printError("Product not found!");
                return;
            }
            
            std::cout << "Current product info:\n";
            std::cout << "Name: " << product_info[0][0] << "\n";
            std::cout << "Price: " << product_info[0][1] << "\n";
            std::cout << "Stock: " << product_info[0][2] << "\n";
            
            std::string new_name = UI::getStringInput("Enter new name (leave empty to keep current): ", true);
            double new_price = UI::getDoubleInput("Enter new price (enter -1 to keep current): ", -1);
            int new_quantity = UI::getInput<int>("Enter new quantity (enter -1 to keep current): ", -1);
            
            std::string update_query = "UPDATE products SET ";
            bool has_update = false;
            
            if (!new_name.empty() && new_name != product_info[0][0]) {
                update_query += "name = '" + new_name + "' ";
                has_update = true;
            }
            
            if (new_price >= 0 && new_price != std::stod(product_info[0][1])) {
                if (has_update) update_query += ", ";
                update_query += "price = " + std::to_string(new_price) + " ";
                has_update = true;
            }
            
            if (new_quantity >= 0 && new_quantity != std::stoi(product_info[0][2])) {
                if (has_update) update_query += ", ";
                update_query += "stock_quantity = " + std::to_string(new_quantity) + " ";
                has_update = true;
            }
            
            if (has_update) {
                update_query += " WHERE product_id = " + std::to_string(product_id);
                db->executeNonQuery(update_query);
                
                logger->logAudit("product", product_id, "update", 
                            "Product updated by admin");
                UI::printSuccess("Product updated successfully!");
            } else {
                UI::printInfo("No changes made.");
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error updating product: " + std::string(e.what()));
        }
    }

    void handleDeleteProduct(std::shared_ptr<DatabaseConnection<>> db,
                                    std::shared_ptr<Logger> logger) {
        UI::printSection("Delete Product");
        
        int product_id = UI::getInput<int>("Enter product ID to delete: ", 1);
        
        try {
            auto product_check = db->executeQuery(
                "SELECT name FROM products WHERE product_id = " + std::to_string(product_id)
            );
            
            if (product_check.empty()) {
                UI::printError("Product not found!");
                return;
            }
            
            std::string product_name = product_check[0][0];
            
            auto order_check = db->executeQuery(
                "SELECT COUNT(*) FROM order_items WHERE product_id = " + std::to_string(product_id)
            );
            
            if (!order_check.empty() && std::stoi(order_check[0][0]) > 0) {
                UI::printError("Cannot delete product. It is used in " + order_check[0][0] + " orders!");
                return;
            }
            
            std::cout << "Delete product: " << product_name << "? (yes/no): ";
            std::string confirm;
            std::cin >> confirm;
            UI::clearInputBuffer();
            
            if (confirm == "yes" || confirm == "y") {
                std::string delete_query = "DELETE FROM products WHERE product_id = " + 
                                        std::to_string(product_id);
                db->executeNonQuery(delete_query);
                
                logger->logAudit("product", product_id, "delete", 
                            "Product deleted by admin");
                UI::printSuccess("Product deleted successfully!");
            } else {
                UI::printInfo("Deletion cancelled.");
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error deleting product: " + std::string(e.what()));
        }
    }

    void handleOrderDetails(std::shared_ptr<DatabaseConnection<>> db) {
        UI::printSection("Order Details");
        
        int order_id = UI::getInput<int>("Enter order ID: ", 1);
        
        try {
            auto order_info = db->executeQuery(
                "SELECT o.order_id, u.name, o.status, o.total_price, o.order_date, u.email "
                "FROM orders o JOIN users u ON o.user_id = u.user_id "
                "WHERE o.order_id = " + std::to_string(order_id)
            );
            
            if (order_info.empty()) {
                UI::printError("Order not found!");
                return;
            }
            
            std::cout << "\nOrder Information:\n";
            std::cout << "Order ID: " << order_info[0][0] << "\n";
            std::cout << "Customer: " << order_info[0][1] << "\n";
            std::cout << "Email: " << order_info[0][5] << "\n";
            std::cout << "Status: " << order_info[0][2] << "\n";
            std::cout << "Total: " << order_info[0][3] << " rub\n";
            std::cout << "Date: " << order_info[0][4] << "\n";
            
            auto order_items = db->executeQuery(
                "SELECT p.name, oi.quantity, oi.price, (oi.quantity * oi.price) as total "
                "FROM order_items oi JOIN products p ON oi.product_id = p.product_id "
                "WHERE oi.order_id = " + std::to_string(order_id)
            );
            
            if (!order_items.empty()) {
                std::cout << "\nOrder Items:\n";
                std::cout << std::left << std::setw(30) << "Product"
                        << std::setw(10) << "Qty"
                        << std::setw(15) << "Price"
                        << std::setw(15) << "Total" << "\n";
                std::cout << std::string(70, '-') << "\n";
                
                for (const auto& item : order_items) {
                    std::cout << std::left << std::setw(30) << item[0].substr(0, 28)
                            << std::setw(10) << item[1]
                            << std::setw(15) << item[2]
                            << std::setw(15) << item[3] << "\n";
                }
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
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
            
            std::cout << "Available statuses: pending, completed, canceled, returned\n";
            std::string new_status = UI::getStringInput("Enter new status: ");
            
            std::vector<std::string> valid_statuses = {"pending", "completed", "canceled", "returned"};
            if (std::find(valid_statuses.begin(), valid_statuses.end(), new_status) == valid_statuses.end()) {
                UI::printError("Invalid status!");
                return;
            }
            
            std::string update_query = "UPDATE orders SET status = '" + new_status + 
                                    "' WHERE order_id = " + std::to_string(order_id);
            db->executeNonQuery(update_query);
            
            logger->logOrderStatusChange(order_id, current_status, new_status);
            logger->logAudit("order", order_id, "update", 
                        "Status changed by admin to: " + new_status);
            
            UI::printSuccess("Order status updated!");
            
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
            std::cout << std::left << std::setw(20) << "Old Status"
                    << std::setw(20) << "New Status"
                    << std::setw(25) << "Changed At"
                    << std::setw(20) << "Changed By" << "\n";
            std::cout << std::string(85, '-') << "\n";
            
            for (const auto& record : history) {
                if (record.size() >= 5) {
                    std::cout << std::left << std::setw(20) << record[2]
                            << std::setw(20) << record[3]
                            << std::setw(25) << record[4]
                            << std::setw(20) << (record.size() > 5 ? record[5] : "N/A") << "\n";
                }
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
        }
    }

    void handleAuditLogs(std::shared_ptr<DatabaseConnection<>> db,
                                std::shared_ptr<Logger> logger) {
        UI::printSection("Audit Logs");
        
        try {
            auto audit_logs = logger->getAllAuditLogs();
            
            if (audit_logs.empty()) {
                UI::printInfo("No audit logs found.");
                return;
            }
            
            std::cout << "Recent audit logs (max 50):\n";
            std::cout << std::left << std::setw(15) << "Entity"
                    << std::setw(10) << "ID"
                    << std::setw(15) << "Operation"
                    << std::setw(25) << "Time"
                    << std::setw(20) << "User"
                    << "Details" << "\n";
            std::cout << std::string(100, '-') << "\n";
            
            for (const auto& log : audit_logs) {
                if (log.size() >= 6) {
                    std::cout << std::left << std::setw(15) << (log[1].size() > 14 ? log[1].substr(0, 14) : log[1])
                            << std::setw(10) << log[2]
                            << std::setw(15) << (log[3].size() > 14 ? log[3].substr(0, 14) : log[3])
                            << std::setw(25) << (log[5].size() > 24 ? log[5].substr(0, 24) : log[5])
                            << std::setw(20) << (log[6].size() > 19 ? log[6].substr(0, 19) : log[6])
                            << (log.size() > 7 ? (log[7].size() > 40 ? log[7].substr(0, 40) + "..." : log[7]) : "") << "\n";
                }
            }
            
        } catch (const std::exception& e) {
            UI::printError("Error: " + std::string(e.what()));
        }
    }

    void handleGenerateReport(std::shared_ptr<DatabaseConnection<>> db,
                                        std::shared_ptr<Logger> logger) {
        UI::printSection("Generate Report");
        
        std::cout << "Generating CSV report\n";
        
        if (logger->generateCSVReport("reports/audit_report.csv")) {
            UI::printSuccess("Report generated successfully: reports/audit_report.csv");
        } else {
            UI::printError("Failed to generate report!");
        }
    }
}

