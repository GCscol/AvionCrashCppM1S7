#include "Environnement.h"
#include "Constantes.h"
#include <cmath>
#include <algorithm>

double Environnement::calculer_rho(double altitude) const {
    altitude = std::max(0.0, altitude);

    // Altitude de transition (tropopause) où la décroissance linéaire de
    // température s'arrête et devient isotherme (valeur standard ~ 11000 m)
    const double h_tropo = (T_0 - T_min) / L;

    if (altitude <= h_tropo) {
        double T = T_0 - L * altitude;
        return rho_0 * std::pow(T / T_0, Physique::g / (R * L) - 1);
    } else {
        // Pour la couche isotherme (alt > h_tropo) : densité décroît
        // exponentiellement avec la loi barométrique
        double T = T_min;
        double rho_at_ht = rho_0 * std::pow(T / T_0, Physique::g / (R * L) - 1);
        return rho_at_ht * std::exp(-Physique::g / (R * T) * (altitude - h_tropo));
    }
}

double Environnement::calculer_vitesse_son(double altitude) const {
    return 340.0 - 0.0065 * altitude;    /// + ça serait cool de la mettre en vrai constante physique
}
