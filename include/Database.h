#pragma once


#include <pqxx/pqxx>
#include <vector>
#include <memory>
#include <string>


template<typename ConnectionString = std::string>

class DatabaseConnection {
    private:
        std::unique_ptr<pqxx::connection> conn;
        std::unique_ptr<pqxx::work> transaction;

    public:
        DatabaseConnection(const ConnectionString& conn_str){
            try {
                conn = std::make_unique<pqxx::connection>(std::string(conn_str));

                if (!conn -> is_open()){
                    throw std::runtime_error("Failed to open db connection");
                }
                std::cout << "Connected to db: " << conn -> dbname() << "\n";
            } catch (const std::exception& e){
                throw std::runtime_error(std::string("Database connection error: ") + e.what());
            }
        } 

        ~DatabaseConnection() = default;

        std::string getTransactionStatus() {
            if (transaction) return "ACTIVE";
            return "INACTIVE";
        }

        void createTrigger(const std::string& SQLtrigger) { executeNonQuery(SQLtrigger); }
        void createFunction(const std::string& SQLfunction) { executeNonQuery(SQLfunction); }

        void rollbackTransaction() {
            if (transaction) {
                transaction -> abort();
                transaction.reset();
            }
        }

        void commitTransaction(){
            if (transaction){
                transaction -> commit();
                transaction.reset();
            }
        }

        void beginTransaction(){
            if (!transaction) transaction = std::make_unique<pqxx::work>(*conn);
        }

        void executeNonQuery(const std::string& query){
            if (transaction){
                transaction -> exec(query);
            } else{
                pqxx::work wrk(*conn);
                wrk.exec(query);
                wrk.commit();
            }
        }
        
        std::vector<std::vector<std::string>> executeQuery(const std::string& query){
            std::vector<std::vector<std::string>> res;

            if (transaction) {
                pqxx::res r = transaction -> exec(query);

                for (const auto& row : r){
                    std::vector<std::string> rowData;
                    for (const auto& f : row){
                        rowData.push_back(f.c_str());
                    }
                    res.push_back(rowData);
                }
            } else {
                pqxx::nontransaction nontr(*conn);
                pqxx::res r = nontr.exec(query);

                for (const auto& row : r){
                    std::vector<std::string> rowData;
                    for (const auto& f : row){
                        rowData.push_back(f.c_str());
                    }
                    res.push_back(rowData);
                }
            }

            return res;
        }
}