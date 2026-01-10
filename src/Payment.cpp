#include "Payment.h"
#include <thread>
#include <chrono>


bool CreditCardPayment::processPayment(double amount) {
    std::cout << "\n=== Pay by bank card ===\n";
    std::cout << "Summa: " << amount << " rub\n";
    
    std::cout << "1. Connect to pay system\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "2. Check card data\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "3. Approved\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "Payment is complete!\n";
    return true;
}


bool EWalletPayment::processPayment(double amount) {
    std::cout << "\n=== Pay by ewallet ===\n";
    std::cout << "Summa: " << amount << " rub\n";
    
    std::cout << "1. Redirection\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "2. Waiting for confirmation\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "3. Check transaction\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "Money debited!\n";
    return true;
}

bool SBPPayment::processPayment(double amount) {
    std::cout << "\n=== Pay by SBP ===\n";
    std::cout << "Summa: " << amount << " rub\n";
    
    std::cout << "1. Generate qr-code\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "2. Waiting\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "3. Approving\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::cout << "Money debited!\n";
    return true;
}


Payment::Payment(double am) 
    : amount(am), paid(false), strategy(nullptr) {}

Payment::Payment(double am, std::unique_ptr<PaymentStrategy> strategy) 
    : amount(am), paid(false), strategy(std::move(strategy)) {}

void Payment::setStrategy(std::unique_ptr<PaymentStrategy> newStrategy) {
    strategy = std::move(newStrategy);
}

bool Payment::process() {
    if (!strategy) {
        std::cerr << "Error: no strategy!\n";
        return false;
    }
    
    std::cout << "\nStart processing\n";
    std::cout << "Payment method: " << strategy->getName() << "\n";
    std::cout << "Sum to be paid: " << amount << " rub\n";
    
    paid = strategy->processPayment(amount);
    
    if (paid) {
        std::cout << "\nFinished!\n";
    } else {
        std::cerr << "\n Not finished\n";
    }
    
    return paid;
}


bool Payment::isPaid() const { return paid; }


std::string Payment::getPaymentInfo() const {
    return "Summa: " + std::to_string(amount) + 
           " rub, status: " + (paid ? "Paid" : "Didn`t pay") +
           ", method: " + getStrategyName();
}

std::string Payment::getStrategyName() const {
    return strategy ? strategy->getName() : "Didn`t choose";
}