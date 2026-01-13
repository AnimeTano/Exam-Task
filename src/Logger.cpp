#include "Logger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>


Logger::Logger(std::shared_ptr<DatabaseConnection<>> database) 
    : db(database), cur_user_id(0) {
    if (!db) {
        std::cerr << "Logger: Database connection is null!\n";
    }
}


void Logger::setUser(int user_id) {
    cur_user_id = user_id;
    std::cout << "Current user set to ID: " << user_id << "\n";
}


void Logger::logToDatabase(
    const std::string& entity_type,
    int entity_id,
    const std::string& operation,
    const std::string& details
) {
    if (!db) {
        std::cerr << "No database connection for logging!\n";
        return;
    }
    
    try {
        std::string query = 
            "INSERT INTO audit_log (entity_type, entity_id, operation, performed_by, performed_at, details) "
            "VALUES ('" + entity_type + "', " + std::to_string(entity_id) + 
            ", '" + operation + "', " + std::to_string(cur_user_id) + 
            ", CURRENT_TIMESTAMP, '" + details + "')";
        
        db->executeNonQuery(query);
        
    } catch (const std::exception& e) {
        std::cerr << "Logger failed to write to database: " << e.what() << "\n";
    }
}


void Logger::logOrderStatusToDatabase(
    int order_id,
    const std::string& old_status,
    const std::string& new_status
) {
    if (!db) return;
    
    try {
        std::string query = 
            "INSERT INTO order_status_history (order_id, old_status, new_status, changed_at, changed_by) "
            "VALUES (" + std::to_string(order_id) + ", '" + old_status + 
            "', '" + new_status + "', CURRENT_TIMESTAMP, " + 
            std::to_string(cur_user_id) + ")";
        
        db->executeNonQuery(query);
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to log order status to DB: " << e.what() << "\n";
    }
}


void Logger::logAudit(
    const std::string& entity_type,
    int entity_id,
    const std::string& operation,
    const std::string& details
) {
    std::cout << "Audit: "<< entity_type << " #" << entity_id 
              << " " << operation << " - " << details << "\n";
    
    logToDatabase(entity_type, entity_id, operation, details);
}


void Logger::logOrderStatusChange(
    int order_id,
    const std::string& current_status,
    const std::string& new_status
) {
    std::cout << "Order #" << order_id << " changed from " 
              << current_status << " to " << new_status << "\n";
    
    logOrderStatusToDatabase(order_id, current_status, new_status);
    
    logAudit("order", order_id, "update", "Status changed from " + current_status + " to " + new_status);
}


std::vector<std::vector<std::string>> Logger::getAudit(int user_id) {
    std::vector<std::vector<std::string>> result;
    
    if (!db) return result;
    
    try {
        std::string query = 
            "SELECT al.log_id, al.entity_type, al.entity_id, al.operation, "
            "u.name as performer, al.performed_at, al.details "
            "FROM audit_log al "
            "LEFT JOIN users u ON al.performed_by = u.user_id ";
        
        if (user_id > 0) {
            query += "WHERE al.performed_by = " + std::to_string(user_id) + " ";
        }
        
        query += "ORDER BY al.performed_at DESC LIMIT 50";
        
        result = db->executeQuery(query);
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to get audit logs: " << e.what() << "\n";
    }
    
    return result;
}


std::vector<std::vector<std::string>> Logger::getOrderStatusHistory(int order_id) {
    std::vector<std::vector<std::string>> result;
    
    if (!db) return result;
    
    try {
        std::string query = 
            "SELECT osh.history_id, osh.order_id, osh.old_status, osh.new_status, "
            "osh.changed_at, u.name as changed_by_name "
            "FROM order_status_history osh "
            "LEFT JOIN users u ON osh.changed_by = u.user_id "
            "WHERE osh.order_id = " + std::to_string(order_id) + " "
            "ORDER BY osh.changed_at DESC";
        
        result = db->executeQuery(query);
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to get order status history: " << e.what() << "\n";
    }
    
    return result;
}


std::vector<std::vector<std::string>> Logger::getAllAuditLogs() {
    return getAudit(-1);
}


std::vector<std::vector<std::string>> Logger::getAuditByUser(int user_id) {
    return getAudit(user_id);
}


bool Logger::generateCSVReport(const std::string& filename) {
    if (!db) {
        std::cerr << "No database connection for report generation\n";
        return false;
    }
    
    try {
        std::string query = 
            "SELECT "
            "  o.order_id, "
            "  u.name as customer_name, "
            "  o.status as order_status, "
            "  o.total_price, "
            "  o.order_date, "
            "  osh.old_status, "
            "  osh.new_status, "
            "  osh.changed_at as status_change_date, "
            "  uc.name as changed_by_user, "
            "  al.operation as audit_operation, "
            "  al.performed_at as audit_date, "
            "  al.details "
            "FROM orders o "
            "LEFT JOIN users u ON o.user_id = u.user_id "
            "LEFT JOIN order_status_history osh ON o.order_id = osh.order_id "
            "LEFT JOIN users uc ON osh.changed_by = uc.user_id "
            "LEFT JOIN audit_log al ON o.order_id = al.entity_id AND al.entity_type = 'order' "
            "ORDER BY o.order_date DESC, osh.changed_at DESC";
        
        auto result = db->executeQuery(query);
        
        if (result.empty()) {
            std::cout << "No data for report\n";
            return false;
        }
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open file for writing: " << filename << "\n";
            return false;
        }
        
        file << "order_id,customer_name,order_status,total_price,order_date,"
             << "old_status,new_status,status_change_date,changed_by_user,"
             << "audit_operation,audit_date,details\n";
        
        for (size_t i = 0; i < result.size(); ++i) {
            for (size_t j = 0; j < result[i].size(); ++j) {
                std::string cell = result[i][j];
                if (cell.find(',') != std::string::npos || 
                    cell.find('"') != std::string::npos) {
                    size_t pos = 0;
                    while ((pos = cell.find('"', pos)) != std::string::npos) {
                        cell.replace(pos, 1, "\"\"");
                        pos += 2;
                    }
                    cell = "\"" + cell + "\"";
                }
                file << cell;
                if (j < result[i].size() - 1) file << ",";
            }
            file << "\n";
        }
        
        file.close();
        std::cout << "CSV report saved to: " << filename << "\n";
        std::cout << "Records exported: " << result.size() << "\n";
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to generate CSV report: " << e.what() << "\n";
        return false;
    }
}


std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
