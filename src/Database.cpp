#include "Database.h"


namespace DatabaseUtils {
    bool tableExists(DatabaseConnection<>& db, const std::string& table_name) {
        try {
            std::string query = 
                "SELECT EXISTS ("
                "   SELECT FROM information_schema.tables "
                "   WHERE table_schema = 'public' "
                "   AND table_name = '" + table_name + "'"
                ");";
            
            auto result = db.executeQuery(query);
            
            if (!result.empty() && !result[0].empty()) {
                return result[0][0] == "t";
            }
            return false;
            
        } catch (...) {
            return false;
        }
    }
    
    std::vector<std::string> getTables(DatabaseConnection<>& db) {
        std::vector<std::string> tables;
        try {
            auto result = db.executeQuery(
                "SELECT table_name FROM information_schema.tables "
                "WHERE table_schema = 'public' ORDER BY table_name;"
            );
            
            for (const auto& row : result) {
                if (!row.empty()) {
                    tables.push_back(row[0]);
                }
            }
        } catch (...) {
        }
        return tables;
    }
}