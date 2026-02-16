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

    alpha_stall_up   = 16.0 * DEG_TO_RAD;  // Hystérésis réaliste transport aircraft
    alpha_stall_down = 12.0 * DEG_TO_RAD;

    tau_sep = 0.15;  // Temps caractéristiques
    tau_att = 0.60;

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
    // ============================
    // MODELE LINEAIRE EXISTANT
    // ============================

    double CL_att =
        5.0 * (alpha - alpha0)
        + 0.44 * delta_p;

    // ============================
    // MODELE POST-STALL
    // ============================

    double CL_sep =
        CL_max * sin(2.0 * alpha);

    // ============================
    // EQUILIBRE DE SEPARATION
    // ============================

    double X_eq =
        0.5 *
        (1.0 +
         tanh((alpha - alpha_stall_up) / delta_alpha));

    // ============================
    // DYNAMIQUE HYSTERESIS
    // ============================

    if(alpha > alpha_stall_up)
    {
        X_sep += dt * (X_eq - X_sep) / tau_sep;
    }
    else if(alpha < alpha_stall_down)
    {
        X_sep += dt * (X_eq - X_sep) / tau_att;
    }
    // sinon hystérésis pure (mémoire)

    clamp_state();

    // ============================
    // INTERPOLATION PHYSIQUE
    // ============================

    C_L =
        (1.0 - X_sep) * CL_att
        + X_sep * CL_sep;

    // ============================
    // TRAINEE REALISTE
    // ============================

    C_D =
        0.0175
        + 0.055 * C_L * C_L
        + 0.25 * X_sep;

    // ============================
    // MOMENT (identique modèle existant + effet stall)
    // ============================

    C_m =
        -0.1
        - (alpha - alpha0)
        - 1.46 * delta_p
        - 12.0 * omega * get_corde() / vitesse
        - 0.6 * X_sep;
}
