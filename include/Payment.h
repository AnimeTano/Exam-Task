#pragma once


#include <string>


class Payment{
    private:
        double amount;
        bool paid;

    public:
        Payment(double am) : amount(am), paid(false) {}

        void process() { paid = true; }
        bool isPaid() const { return paid; }
};