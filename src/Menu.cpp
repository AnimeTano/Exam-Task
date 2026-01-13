#include "Menu.h"
#include "AdminMenu.h"
#include "ManagerMenu.h"
#include "CustomerMenu.h"


void displayAdminMenu(std::shared_ptr<Admin> admin,  std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger) {
    AdminMenu::handleAdminMenu(admin, db, logger);
}

void displayManagerMenu(std::shared_ptr<Manager> manager, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger) {
    ManagerMenu::handleManagerMenu(manager, db, logger);
}

void displayCustomerMenu(std::shared_ptr<Customer> customer, std::shared_ptr<DatabaseConnection<>> db, std::shared_ptr<Logger> logger) {
    CustomerMenu::handleCustomerMenu(customer, db, logger);
}