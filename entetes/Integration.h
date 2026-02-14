#ifndef INTEGRATION_H
#define INTEGRATION_H

#include <cmath>

class Avion;

class Integration {
    
public:
    static void Euler(Avion& avion, double dt);
    static void RK4(Avion& avion, double dt);

private:

    struct Derivees { // Struct pour RK4 et Euler  (peut etre plus opti aveec un vecteur ou autre, juste le plus simple/pratique au départ c'est un struct et peut etre plus opti de ne pas créer la fct calcul dérivée)
        double dx, dy, dz;
        double dvx, dvy, dvz;
        double dpitch, domega_pitch;
    };

    static Derivees calculer_derivees(Avion& avion);

};

#endif // INTEGRATION_H