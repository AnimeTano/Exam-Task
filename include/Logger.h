#pragma once 


#include <string>
#include <memory>
#include <vector>
#include "Database.h"


class Logger {
    private:
        std::shared_ptr<DatabaseConnection<>> db;
        int cur_user_id;
        
        void logToDatabase(
            const std::string& entity_type,
            int entity_id,
            const std::string& operation,
            const std::string& details
        );
        
        void logOrderStatusToDatabase(
            int order_id,
            const std::string& old_status,
            const std::string& new_status
        );

    public:
        Logger(std::shared_ptr<DatabaseConnection<>> database);

        void setUser(int user_id);
        int getCurrentUserId() const { return cur_user_id; }

        void logAudit(
            const std::string& entity_type,
            int entity_id,
            const std::string& operation,
            const std::string& details
        );

        void logOrderStatusChange(
            int order_id,
            const std::string& current_status,
            const std::string& new_status
        );

        std::vector<std::vector<std::string>> getAudit(int user_id = -1);
        std::vector<std::vector<std::string>> getOrderStatusHistory(int order_id);
        
        std::vector<std::vector<std::string>> getAllAuditLogs();
        std::vector<std::vector<std::string>> getAuditByUser(int user_id);
        
        bool generateCSVReport(const std::string& filename);
        
        static std::string getCurrentTimestamp();
};