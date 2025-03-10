#ifndef UTILS_H
#define UTILS_H
#include<iostream>
using namespace std;

void displayMenu() {
    cout << "\n======= Deribit Trading System =======\n";
    cout << "1. Place Order\n";
    cout << "2. Modify Order\n";
    cout << "3. Sell Order\n";
    cout << "4. Cancel Order\n";
    cout << "5. Cancel All Orders\n";
    cout << "6. Get Open Orders\n";
    cout << "7. Get Open Orders by Currency\n";
    cout << "8. Get Open Orders by Instrument\n";
    cout << "9. Get Order State\n";
    cout << "10. Get Order History by Currency\n";
    cout << "11. Get Order History by Instrument\n";
    cout << "12. Get User Trades by Currency\n";
    cout << "13. Get User Trades by Instrument\n";
    cout << "14. Exit\n";
    cout << "======================================\n";
    cout << "Enter your choice: ";
}
#endif