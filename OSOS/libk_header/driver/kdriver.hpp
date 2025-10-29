#ifndef _OSOS_DRIVER_KDRIVER_H
#define _OSOS_DRIVER_KDRIVER_H

#define MAX_NUMDRIVERS 265

namespace driver
{
    /// @brief Base class (interface) for all hardware drivers in the operating system.
    class Driver{ 
        private:
            
        public:
            Driver();
            virtual ~Driver();
            virtual void activate()=0;
            virtual int reset()=0;
            virtual void deactivate()=0;
    };

    /// @brief Manages a collection of Driver objects, facilitating their activation and organization.
    class DriverManager{
        private:
            Driver* drivers[MAX_NUMDRIVERS];
            int numDrivers;
        public:
            DriverManager();
            ~DriverManager();
            void addDriver(Driver*);
            void activateAll();
    };
}

#endif