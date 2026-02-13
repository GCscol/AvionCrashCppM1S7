#ifndef ETAT_CINEMATIQUE_H
#define ETAT_CINEMATIQUE_H

#include <cmath>

struct EtatCinematique {
     // Position dans l'espace (en m)
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    
     // Vitesse (en m/s)
    double vx = 0.0;
    double vy = 0.0;
    double vz = 0.0;
    
     // Orientation (en radians) (Euler)
    double roll = 0.0;
    double pitch = 0.0;
    double yaw = 0.0;
    
    double omega_pitch = 0.0;


    EtatCinematique();


    double get_vitesse_norme() const;
    double get_gamma() const;
    double get_alpha() const;
};

#endif // ETAT_CINEMATIQUE_H
 
