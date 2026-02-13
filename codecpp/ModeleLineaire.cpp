#include "ModeleLineaire.h"
#include "Constantes.h"

ModeleLineaire::ModeleLineaire(double surface, double corde, double delta_prof_init)
    : ModeleAerodynamique(surface, corde, delta_prof_init) {}

void ModeleLineaire::update_from_polar(double alpha_rad, double delta_p, 
                                       double omega, double vitesse) {
    using namespace Physique;
    double alpha_deg = alpha_rad * RAD_TO_DEG;
    
    // Portance avec gestion du décrochage
    if (alpha_deg <= 15.0) {  // Régime linéaire avant décrochage
        C_L = 5.0 * (alpha_rad - (-0.035)) + 0.44 * delta_p;
    } else if (alpha_deg <= 20.0) { // Décroissance linéaire après décrochage
        double alpha_stall_rad = 8.0 * DEG_TO_RAD;
        double CL_max = 5.0 * (alpha_stall_rad - (-0.035)) + 0.44 * delta_p;
        C_L = CL_max * (1.0 - 0.1 * (alpha_deg - 8.0));
    } else {
        C_L = 0.1;  // Portance résiduelle ||||A MODIFIER de sorte à ce que la polaire soit continue|||||||||||||||||||||||||||||||||||
    }
      //// Traînée parabolique
    C_D = 0.0175 + 0.055 * C_L * C_L;    
    
     /// Moment de tangage
    C_m = -0.1 - (alpha_rad - (-0.035)) - 1.46 * delta_p + (-12.0) * omega * l / vitesse;
}
