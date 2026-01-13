#pragma once


#include "UI.h"
#include "Database.h"
#include "User.h"
#include "Order.h"
#include "Payment.h"
#include "Logger.h"
#include <memory>


namespace CustomerMenu {
    void handleCustomerMenu(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleCreateOrder(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleAddToOrder(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db);
    
    void handleRemoveFromOrder(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db);
    
    void handleViewMyOrders(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db);
    
    void handleViewOrderStatus(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db);
    
    void handleMakePayment(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleReturnOrder(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleOrderStatusHistory(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void showAvailableProducts(std::shared_ptr<DatabaseConnection<>> db);
    
    std::unique_ptr<PaymentStrategy> selectPaymentStrategy();
    
    void processOrderCreation(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
}