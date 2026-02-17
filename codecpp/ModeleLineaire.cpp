#include "ModeleLineaire.h"
#include "Constantes.h"

ModeleLineaire::ModeleLineaire(double surface, double corde, double delta_prof_init)
    : ModeleAerodynamique(surface, corde, delta_prof_init) {}

void ModeleLineaire::update_from_polar(double alpha_rad, double delta_p, 
                                       double omega, double vitesse) {
    using namespace Physique;
    double alpha_deg = alpha_rad * RAD_TO_DEG;
    
    // Linear lift before stall
    if (alpha_deg <= 15.0 && alpha_deg >= -10) {
        C_L = 5.0 * (alpha_rad - (-0.035)) + 0.44 * delta_p;
    } else {
        C_L = 0;
    }
    
    // Parabolic drag
    C_D = 0.0175 + 0.055 * C_L * C_L;    
    
    // Pitching moment
    C_m = -0.1 - (alpha_rad - (-0.035)) - 1.46 * delta_p + (-12.0) * omega * l / vitesse;
}
