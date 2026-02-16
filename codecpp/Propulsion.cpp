#include "Propulsion.h"
#include <cmath>
#include "Environnement.h"
#include <algorithm>

Propulsion::Propulsion(double poussee_nominale, int nb_moteurs, double expo_alt)
    : F0(poussee_nominale), n_moteurs(nb_moteurs), expo_alt(expo_alt) {}

// double Propulsion::calculer_poussee_max(double vitesse, double rho, double altitude) const
double Propulsion::calculer_poussee_max(double, double rho, double) const {
    double sigma = rho / 1.225;  // Densité relative à intégrer via environnement
    double F_altitude = n_moteurs * F0 * std::pow(sigma, expo_alt);
    // // Corrections pour vitesse et Mach (valeurs conservatrices)
    // double F_speed_correction = 1.0 - 0.25 * (vitesse / 300.0); // diminue avec la vitesse
    // if (F_speed_correction < 0.25) F_speed_correction = 0.25;   // ne descend pas en dessous de 25%

    // double speed_of_sound = 340.0 - 0.0065 * altitude; // approximation
    // double Mach = vitesse / std::max(1e-3, speed_of_sound);
    // double F_mach_correction = 1.0;
    // if (Mach > 0.8) {
    //     F_mach_correction = 1.0 - 0.5 * (Mach - 0.8);
    // }
    // if (Mach > 1.2) {
    //     F_mach_correction = 0.4;
    // }

    return F_altitude; // * F_speed_correction * F_mach_correction;
}
