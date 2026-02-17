#include "ModeleAerodynamique.h"

ModeleAerodynamique::ModeleAerodynamique(double surface, double corde, double delta_prof_init)
    : S(surface), l(corde), delta_profondeur(delta_prof_init),
      C_L(0.0), C_D(0.0), C_m(0.0), omega_pitch(0.0) {}

double ModeleAerodynamique::get_surface() const {
    return S;
}

double ModeleAerodynamique::get_corde() const {
    return l;
}

double ModeleAerodynamique::get_delta_profondeur() const {
    return delta_profondeur;
}

void ModeleAerodynamique::set_delta_profondeur(double val) {
    delta_profondeur = val;
}

double ModeleAerodynamique::calculer_portance(double vitesse, double rho) const {
    return 0.5 * rho * vitesse * vitesse * S * C_L;
}

double ModeleAerodynamique::calculer_trainee(double vitesse, double rho) const {
    return 0.5 * rho * vitesse * vitesse * S * C_D;
}

double ModeleAerodynamique::calculer_moment_pitch(double vitesse, double rho) const {
    return 0.5 * rho * vitesse * vitesse * S * C_m * l;
}
