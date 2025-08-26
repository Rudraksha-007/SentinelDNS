#include "logger.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
void logMessage(const string& message) {
    ofstream logFile("dns_server.log", ios_base::app); // Open log file in append mode
    if (logFile.is_open()) {
        logFile << message << endl;
        logFile.close();
    } else {
        cerr << "Unable to open log file!" << endl; // Error handling
    }
}