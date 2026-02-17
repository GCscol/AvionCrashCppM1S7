#include "ModeleHysteresis.h"
#include "Constantes.h"
#include <cmath>

using namespace Physique;

ModeleHysteresis::ModeleHysteresis(double surface, double corde, double delta_prof_init)
:
ModeleAerodynamique(surface, corde, delta_prof_init),
X_sep(0.0)
{
    alpha0 = -0.035;

    alpha_stall_up   = 16.0 * DEG_TO_RAD;  // Realistic transport aircraft
    alpha_stall_down = 12.0 * DEG_TO_RAD;

    tau_sep = 0.5;   // Smoother transition to stall
    tau_att = 1.5;   // Smoother reattachment

    delta_alpha = 3.0 * DEG_TO_RAD; // largeur transition

    CL_max = 1.45;  // CL max A330
}

void ModeleHysteresis::clamp_state()
{
    if (X_sep < 0.0) X_sep = 0.0;
    if (X_sep > 1.0) X_sep = 1.0;
}

void ModeleHysteresis::update_from_polar(double alpha,
                                         double delta_p,
                                         double omega,
                                         double vitesse,
                                         double dt)
{
    // Linear attached flow model
    double CL_att =
        5.0 * (alpha - alpha0)
        + 0.44 * delta_p;

    // Post-stall model: CL recovers near stall, decays at extreme angles
    double alpha_stall_mid = (alpha_stall_up + alpha_stall_down) / 2.0;
    
    double CL_sep;
    if (alpha >= alpha_stall_up) {
        double alpha_excess = alpha - alpha_stall_up;
        double decay_factor = exp(-0.30 * alpha_excess * alpha_excess / (delta_alpha * delta_alpha));
        CL_sep = CL_max * 0.5 * decay_factor;
    } else if (alpha <= alpha_stall_down) {
        double alpha_excess = alpha_stall_down - alpha;
        double decay_factor = exp(-0.30 * alpha_excess * alpha_excess / (delta_alpha * delta_alpha));
        CL_sep = -CL_max * 0.5 * decay_factor;
    } else {
        CL_sep = 0.5 * CL_max * sin(2.0 * (alpha - alpha_stall_mid));
    }

    // Separation equilibrium
    double X_eq =
        0.5 *
        (1.0 +
         tanh((alpha - alpha_stall_up) / delta_alpha));

    // Hysteresis dynamics
    if(alpha > alpha_stall_up)
    {
        X_sep += dt * (X_eq - X_sep) / tau_sep;
    }
    else if(alpha < alpha_stall_down)
    {
        X_sep += dt * (X_eq - X_sep) / tau_att;
    }

    clamp_state();

    // Interpolate between attached and separated flow
    C_L =
        (1.0 - X_sep) * CL_att
        + X_sep * CL_sep;

    // Drag model
    C_D =
        0.0175
        + 0.055 * C_L * C_L
        + 0.25 * X_sep;

    // Moment with stall effect
    C_m =
        -0.1
        - (alpha - alpha0)
        - 1.46 * delta_p
        - 12.0 * omega * get_corde() / vitesse
        - 0.6 * X_sep;
}
