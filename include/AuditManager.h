#pragma once


#include "Logger.h"
#include <memory>


class AuditManager {
    private:
        static std::shared_ptr<Logger> logger;
    
    public:
        static void initialize(std::shared_ptr<DatabaseConnection<>> db);
        static std::shared_ptr<Logger> getLogger();
        
        static void logProductChange(int product_id, const std::string& operation, const std::string& details);
        static void logOrderChange(int order_id, const std::string& operation, const std::string& details);
        static void logUserChange(int user_id, const std::string& operation, const std::string& details);
        static void logStatusChange(int order_id, const std::string& old_status, const std::string& new_status);
};