#include "Environnement.h"
#include "Constantes.h"
#include <cmath>
#include <algorithm>

double Environnement::calculer_rho(double altitude) const {
    altitude = std::max(0.0, altitude);

    // Tropopause at ~11000m: linear temperature below, isothermal above
    const double h_tropo = (T_0 - T_min) / L;

    if (altitude <= h_tropo) {
        double T = T_0 - L * altitude;
        return rho_0 * std::pow(T / T_0, config.getDouble("g") / (R * L) - 1);
    } else {
        // Exponential density decay above tropopause
        double T = T_min;
        double rho_at_ht = rho_0 * std::pow(T / T_0, config.getDouble("g") / (R * L) - 1);
        return rho_at_ht * std::exp(-config.getDouble("g") / (R * T) * (altitude - h_tropo));
    }
}

double Environnement::calculer_temperature(double altitude) const {
    altitude = std::max(0.0, altitude);
    const double h_tropo = (T_0 - T_min) / L;
    
    if (altitude <= h_tropo) {
        return T_0 - L * altitude;
    } else {
        return T_min;  // Isothermal above tropopause
    }
}

double Environnement::calculer_vitesse_son(double altitude) const {
    // Speed of sound: a = sqrt(gamma * R * T)
    // For air: gamma = 1.4 (adiabatic index)
    const double gamma = 1.4;
    double T = calculer_temperature(altitude);
    return std::sqrt(gamma * R * T);
}

double Environnement::calculer_mach(double vitesse, double altitude) const {
    double a = calculer_vitesse_son(altitude);
    return vitesse / a;
}
