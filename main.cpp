#include "include/Database.h"
#include "include/User.h"
#include "include/SqlScript.h"
#include "include/AuditManager.h"
#include "include/ConsoleUtils.h"
#include "include/Menu.h"
#include <iostream>
#include <memory>


auto getPassword = []() -> std::string {
    std::ifstream file("bin/config.txt");
    std::string line;
    if (file.fail()) {
        std::cout << "Error reading password from file\n";
        return "";
    }
    file >> line;
    file.close();
    return line;
};


int main() {
    ConsoleUtils::setupConsole();
    std::string password = getPassword();
    
    try {
        std::string conn_str = "host=localhost dbname=store user=postgres password=" + password;
        auto db = std::make_shared<DatabaseConnection<>>(conn_str);
        
        if (!db->isConnected()) {
            UI::printError("Database connection failed!");
            return 1;
        }
        
        auto logger = std::make_shared<Logger>(db);
        AuditManager::initialize(db);
        
        try {
            SqlScript::initializeDatabase(*db, "sql");
            UI::printSuccess("Database initialized");
        } catch (const std::exception& e) {
            UI::printError("Database init warning: " + std::string(e.what()));
        }
        
        logger->setUser(1);
        logger->logAudit("system", 0, "startup", "Application started");
        
        while (true) {
            UI::printSection("Store System - Role Selection");
            std::cout << "1. Login as Administrator\n";
            std::cout << "2. Login as Manager\n";
            std::cout << "3. Login as Customer\n";
            std::cout << "4. Exit\n";
            
            int choice = UI::getInput<int>("Select role (1-4): ", 1, 4);
            
            if (choice == 4) {
                logger->logAudit("system", 0, "shutdown", "Application closed");
                UI::printSuccess("Thank you for using the system!");
                break;
            }
            
            std::shared_ptr<User> user;
            switch (choice) {
                case 1:
                    user = std::make_shared<Admin>(1, "Admin", "admin@store.com", "pass", true);
                    logger->setUser(1);
                    displayAdminMenu(std::dynamic_pointer_cast<Admin>(user), db, logger);
                    break;
                case 2:
                    user = std::make_shared<Manager>(2, "Manager", "manager@store.com", "pass", false);
                    logger->setUser(2);
                    displayManagerMenu(std::dynamic_pointer_cast<Manager>(user), db, logger);
                    break;
                case 3:
                    user = std::make_shared<Customer>(3, "Customer", "customer@store.com", "pass", false);
                    logger->setUser(3);
                    displayCustomerMenu(std::dynamic_pointer_cast<Customer>(user), db, logger);
                    break;
            }
        }
        
    } catch (const std::exception& e) {
        UI::printError("Critical error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}