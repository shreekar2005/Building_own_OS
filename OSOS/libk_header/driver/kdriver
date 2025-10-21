#ifndef _OSOS_DRIVER_KDRIVER_H
    #define _OSOS_DRIVER_KDRIVER_H
    #define MAX_NUMDRIVERS 265
    namespace driver{
        //Driver is an INTERFACE
        class Driver{ 
            private:
                
            public:
                Driver();
                virtual ~Driver();
                virtual void activate()=0;
                virtual int reset()=0;
                virtual void deactivate()=0;
        };

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