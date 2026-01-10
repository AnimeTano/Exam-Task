#pragma once


#include <memory>
#include <string>
#include <functional>
#include <iostream>


class PaymentStrategy {
    public:
        virtual ~PaymentStrategy() = default;
        virtual bool processPayment(double amount) = 0; 
        virtual std::string getName() const = 0;
};


class EWalletPayment : public PaymentStrategy {
    public:
        bool processPayment(double amount) override;
        std::string getName() const override { return "E-Wallet"; }
};


class SBPPayment : public PaymentStrategy {
    public:
        bool processPayment(double amount) override;
        std::string getName() const override { return "SBP"; }
};


class CreditCardPayment : public PaymentStrategy {
    public:
        bool processPayment(double amount) override;
        std::string getName() const override { return "Credit Card"; }
};


class Payment {
    private:
        double amount;
        bool paid;
        std::unique_ptr<PaymentStrategy> strategy;  
    
    public:
        Payment(double am);
        Payment(double am, std::unique_ptr<PaymentStrategy> strategy);
        
        void setStrategy(std::unique_ptr<PaymentStrategy> newStrategy);
        bool process();                    
        bool isPaid() const;
        
        std::string getPaymentInfo() const;
        std::string getStrategyName() const;
        double getAmount() const { return amount; }
};