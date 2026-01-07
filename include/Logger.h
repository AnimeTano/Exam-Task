#pragma once 


#include <string>
#include <memory>
#include "Database.h"


class Logger{
    private:
        std::shared_ptr<DatabaseConnection<>> db;
        int cur_us_id;

    public:
        Logger(std::shared_ptr<DatabaseConnection<>> database);

        void setUser(int user_id);

        void logAudit(
            const std::string& entity_type,
            int entity_id,
            const std::string& operation,
            const std::string& detalis;
        );

        void logOrderStatusChange(
            int order_id,
            const std::string& current_status,
            const std::string& new_status
        );

        std::vector<std::vector<std::string>> getAudit(int user_id);
        std::vector<std::vector<std::string>> getOrderStatus(int order_id);

        bool genetateCSVReport(const std::string& filename);
};