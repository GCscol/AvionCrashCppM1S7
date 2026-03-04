#ifndef MODELE_HYSTERESIS_H
#define MODELE_HYSTERESIS_H

#include "ModeleAerodynamique.h"

class ModeleHysteresis : public ModeleAerodynamique
{
public:

    ModeleHysteresis(double surface,
                     double corde,
                     double delta_prof_init);
    ~ModeleHysteresis() override;

    // Override base interface with mach parameter (ignored for hysteresis, uses dt instead)
    void update_from_polar(double alpha_rad,
                           double delta_p,
                           double omega,
                           double vitesse,
                           double mach = 0.0) override;

private:
    // Internal implementation with explicit time-step for hysteresis dynamics
    void update_from_polar_dt(double alpha_rad,
                              double delta_p,
                              double omega,
                              double vitesse,
                              double dt);

    // Separation state: 0=attached, 1=separated
    double X_sep;

    // Physical parameters
    double alpha0;

    double alpha_stall_up;
    double alpha_stall_down;

    double tau_sep;
    double tau_att;

    double delta_alpha;

    double CL_max;

    // Safety bounds
    void clamp_state();
};

#endif
