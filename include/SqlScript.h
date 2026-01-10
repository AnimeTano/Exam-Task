#pragma once


#include "Database.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>


class SqlScript {
    public:
        static bool executeScript(DatabaseConnection<>& db, const std::string& filepath);
        static bool executeAllScripts(DatabaseConnection<>& db, const std::string& directory);
        static bool initializeDatabase(DatabaseConnection<>& db, const std::string& sqlSir = "sql");
};