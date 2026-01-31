#include "driver/kdriver.hpp"

using namespace driver;

Driver::Driver(){}
Driver::~Driver(){}

DriverManager::DriverManager()
{
    numDrivers=0;
}

DriverManager::~DriverManager(){}

void DriverManager::addDriver(Driver* driver)
{
    drivers[numDrivers]=driver;
    numDrivers++;
}

#include "basic/kiostream.hpp"
using namespace basic;

void DriverManager::activateAll()
{
    for(int i=0; i<numDrivers; i++){
        printf("%p : ", drivers[i]);
        drivers[i]->activate();
    }
}
