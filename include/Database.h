#pragma once


#include <pqxx/pqxx>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <stdexcept>
#include <type_traits>


template<typename ConnectionString = std::string>
class DatabaseConnection {
    private:
        std::unique_ptr<pqxx::connection> conn;
        std::unique_ptr<pqxx::work> transaction;
        std::string current_connection_string;

    public:
        DatabaseConnection(const ConnectionString& conn_str) {
            try {
                current_connection_string = std::string(conn_str);
                conn = std::make_unique<pqxx::connection>(current_connection_string);
                
                if (!conn->is_open()) {
                    throw std::runtime_error("Failed to open database connection");
                }
                
                std::cout << "Connected to database: " << conn->dbname() << "\n";
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Database connection error: ") + e.what());
            }
        }

        ~DatabaseConnection() {
            if (transaction) {
                try {
                    transaction->abort();
                } catch (...) {
                }
            }
        }

        bool isConnected() const { return conn && conn->is_open(); }

        std::string getConnectionInfo() const {
            if (!isConnected()) return "Not connected";
            return std::string("Database: ") + conn->dbname();
        }

        void beginTransaction() {
            if (transaction) {
                throw std::runtime_error("Transaction active");
            }
            transaction = std::make_unique<pqxx::work>(*conn);
        }

        void commitTransaction() {
            if (!transaction) {
                throw std::runtime_error("No active transaction to commit");
            }
            transaction->commit();
            transaction.reset();
        }

        void rollbackTransaction() {
            if (!transaction) {
                throw std::runtime_error("No active transaction to rollback");
            }
            transaction->abort();
            transaction.reset();
        }

        std::string getTransactionStatus() {
            return transaction ? "ACTIVE" : "INACTIVE";
        }

        void executeNonQuery(const std::string& query) {
            try {
                if (transaction) {
                    transaction->exec(query);
                } else {
                    pqxx::work w(*conn);
                    w.exec(query);
                    w.commit();
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Query execution failed: ") + e.what());
            }
        }

        std::vector<std::vector<std::string>> executeQuery(const std::string& query) {
            std::vector<std::vector<std::string>> result;
            
            try {
                if (transaction) {
                    pqxx::result r = transaction->exec(query);
                    convertResult(r, result);
                } else {
                    pqxx::nontransaction nt(*conn);
                    pqxx::result r = nt.exec(query);
                    convertResult(r, result);
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Query execution failed: ") + e.what());
            }
            
            return result;
        }

        std::vector<std::vector<std::string>> executePreparedQuery(
            const std::string& name, 
            const std::string& query,
            const std::string& param = ""
        ) {
            std::vector<std::vector<std::string>> result;
            
            try {
                pqxx::work w(*conn);
                
                std::string final_query = query;
                if (!param.empty()) {
                    size_t pos = final_query.find("$1");
                    if (pos != std::string::npos) {
                        final_query.replace(pos, 2, "'" + param + "'");
                    }
                }
                
                pqxx::result r = w.exec(final_query);
                convertResult(r, result);
                w.commit();
                
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Prepared query failed: ") + e.what());
            }
            
            return result;
        }

        void createFunction(const std::string& function_sql) {
            executeNonQuery(function_sql);
        }

        void createTrigger(const std::string& trigger_sql) {
            executeNonQuery(trigger_sql);
        }

        private:
            void convertResult(const pqxx::result& r, std::vector<std::vector<std::string>>& output) {
                output.clear();
                for (const auto& row : r) {
                    std::vector<std::string> row_data;
                    for (const auto& field : row) {
                        row_data.push_back(field.c_str());
                    }
                    output.push_back(row_data);
                }
            }
};


class TransactionGuard {
    private:
        DatabaseConnection<>& db;
        bool committed;

    public:
        TransactionGuard(DatabaseConnection<>& database) 
            : db(database), committed(false) {
            db.beginTransaction();
        }

        ~TransactionGuard() {
            if (!committed) {
                try {
                    db.rollbackTransaction();
                } catch (...) {
                }
            }
        }

        void commit() {
            db.commitTransaction();
            committed = true;
        }

        void rollback() {
            db.rollbackTransaction();
            committed = true;
        }

        
        TransactionGuard(const TransactionGuard&) = delete;
        TransactionGuard& operator=(const TransactionGuard&) = delete;
};


namespace DatabaseUtils {
    bool tableExists(DatabaseConnection<>& db, const std::string& table_name);
    
    std::vector<std::string> getTables(DatabaseConnection<>& db);
}