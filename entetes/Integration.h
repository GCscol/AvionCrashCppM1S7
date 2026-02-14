#ifndef INTEGRATION_H
#define INTEGRATION_H

#include <cmath>

class Avion;

class Integration {
    
public:
    static void Euler(Avion& avion, double dt);
    static void RK4(Avion& avion, double dt);
};

#endif // INTEGRATION_H