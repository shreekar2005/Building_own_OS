#include "driver/kdriver.hpp"

/// @brief Default constructor for the base Driver class.
driver::Driver::Driver(){}
/// @brief Default destructor for the base Driver class.
driver::Driver::~Driver(){}


/// @brief Constructs a new DriverManager object, initializing the driver count to zero.
driver::DriverManager::DriverManager(){
    numDrivers=0;
}
/// @brief Destroys the DriverManager object.
driver::DriverManager::~DriverManager(){}

/// @brief Adds a driver to the manager's list.
/// @param driver A pointer to the driver object to be added.
void driver::DriverManager::addDriver(driver::Driver* driver){
    drivers[numDrivers]=driver;
    numDrivers++;
}

/// @brief Activates all drivers currently registered with the manager.
void driver::DriverManager::activateAll(){
    for(int i=0; i<numDrivers; i++){
        drivers[i]->activate();
    }
}