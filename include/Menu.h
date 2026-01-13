#pragma once


#include "UI.h"
#include "Database.h"
#include "User.h"
#include "Logger.h"
#include <memory>

void displayAdminMenu(std::shared_ptr<Admin> admin, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);

void displayManagerMenu(std::shared_ptr<Manager> manager, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);

void displayCustomerMenu(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);


namespace OrderOperations {
    void createNewOrder(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void processOrderPayment(int order_id, double amount, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
}


namespace ProductOperations {
    void showAllProducts(std::shared_ptr<DatabaseConnection<>> db);
    void showProductDetails(int product_id, std::shared_ptr<DatabaseConnection<>> db);
}


namespace ReportOperations {
    bool generateOrderReport(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger, const std::string& filename);
}