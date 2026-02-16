#ifndef MODELE_HYSTERESIS_H
#define MODELE_HYSTERESIS_H

#include "ModeleAerodynamique.h"

class ModeleHysteresis : public ModeleAerodynamique
{
public:

    ModeleHysteresis(double surface,
                     double corde,
                     double delta_prof_init);

    // Override base interface (no dt) — forwards to dt-enabled implementation
    void update_from_polar(double alpha_rad,
                           double delta_p,
                           double omega,
                           double vitesse) override
    {
        update_from_polar(alpha_rad, delta_p, omega, vitesse, 0.0);
    }

    // Implementation with explicit time-step for hysteresis dynamics
    void update_from_polar(double alpha_rad,
                           double delta_p,
                           double omega,
                           double vitesse,
                           double dt);

private:

    // Etat interne de séparation (0 = attaché, 1 = séparé)
    double X_sep;

    // Paramètres physiques A330 réalistes
    double alpha0;

    double alpha_stall_up;
    double alpha_stall_down;

    double tau_sep;
    double tau_att;

    double delta_alpha;

    double CL_max;

    // bornes de sécurité
    void clamp_state();
};

#endif
