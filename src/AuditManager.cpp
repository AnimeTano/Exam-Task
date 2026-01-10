#include "AuditManager.h"


std::shared_ptr<Logger> AuditManager::logger = nullptr;


void AuditManager::initialize(std::shared_ptr<DatabaseConnection<>> db) {
    logger = std::make_shared<Logger>(db);
}


std::shared_ptr<Logger> AuditManager::getLogger() {
    if (!logger) {
        throw std::runtime_error("AuditManager not initialized!");
    }
    return logger;
}


void AuditManager::logProductChange(int product_id, const std::string& operation, const std::string& details) {
    if (logger) {
        logger->logAudit("product", product_id, operation, details);
    }
}


void AuditManager::logOrderChange(int order_id, const std::string& operation, const std::string& details) {
    if (logger) {
        logger->logAudit("order", order_id, operation, details);
    }
}


void AuditManager::logUserChange(int user_id, const std::string& operation, const std::string& details) {
    if (logger) {
        logger->logAudit("user", user_id, operation, details);
    }
}


void AuditManager::logStatusChange(int order_id, const std::string& old_status, const std::string& new_status) {
    if (logger) {
        logger->logOrderStatusChange(order_id, old_status, new_status);
    }
}