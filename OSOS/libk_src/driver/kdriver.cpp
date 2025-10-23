#include "driver/kdriver.hpp"

driver::Driver::Driver(){}
driver::Driver::~Driver(){}


driver::DriverManager::DriverManager(){
    numDrivers=0;
}
driver::DriverManager::~DriverManager(){}

void driver::DriverManager::addDriver(driver::Driver* driver){
    drivers[numDrivers]=driver;
    numDrivers++;
}

void driver::DriverManager::activateAll(){
    for(int i=0; i<numDrivers; i++){
        drivers[i]->activate();
    }
}