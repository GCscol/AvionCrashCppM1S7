#ifndef INTEGRATION_H
#define INTEGRATION_H

#include <cmath>

class Avion;

class Integration {
    
public:
    struct Derivees {
        double dx, dy, dz;           // Position derivatives
        double dvx, dvy, dvz;        // Velocity derivatives
        double dpitch;               // Pitch angle derivative
        double domega_pitch;         // Pitch angular velocity derivative
    };

    static Derivees calculer_derivees(Avion& avion);
    static void Euler(Avion& avion, double dt);
    static void RK4(Avion& avion, double dt);
};

#endif // INTEGRATION_H