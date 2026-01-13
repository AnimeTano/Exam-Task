#pragma once


#include "UI.h"
#include "Database.h"
#include "User.h"
#include "Logger.h"
#include <memory>


namespace AdminMenu {
    void handleAdminMenu(std::shared_ptr<Admin> admin, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleAddProduct(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleUpdateProduct(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleDeleteProduct(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleViewAllOrders(std::shared_ptr<DatabaseConnection<>> db);
    
    void handleOrderDetails(std::shared_ptr<DatabaseConnection<>> db);
    
    void handleChangeOrderStatus(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleOrderStatusHistory(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleAuditLogs(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleGenerateReport(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
}