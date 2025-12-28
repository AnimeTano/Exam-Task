#pragma once


#include <string>


class User{
    protected:
        std::string username;
        int id;

    public:
        User(const std::string& name, int i) : username(name), id(i) {}


        std::string getUsername() const { return username; }
        int getId() const { return id; }

        void setUsername(const std::string& name) { username = name; }
        void setId(int i) { id = i; }
};