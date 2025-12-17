#pragma once 


#include <string>


class Logger {
    private:
        void WritetoFile(const std::string& message);

    public:
        static void Log();
}