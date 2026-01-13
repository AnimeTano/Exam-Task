#pragma once


#include "UI.h"
#include "Database.h"
#include "User.h"
#include "Logger.h"
#include <memory>


namespace ManagerMenu {
    void handleManagerMenu(std::shared_ptr<Manager> manager, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handlePendingOrders(std::shared_ptr<DatabaseConnection<>> db);
    
    void handleApproveOrder(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleUpdateStock(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleViewOrderDetails(std::shared_ptr<DatabaseConnection<>> db);
    
    void handleChangeOrderStatus(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    void handleApprovedOrdersHistory(std::shared_ptr<DatabaseConnection<>> db);
    
    void handleOrderStatusHistory(std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger);
    
    bool canManagerChangeStatus(const std::string& current_status, const std::string& new_status);
    
    void showManagerStatistics(std::shared_ptr<DatabaseConnection<>> db);
}