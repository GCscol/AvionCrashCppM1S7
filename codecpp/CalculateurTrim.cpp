#include "CalculateurTrim.h"
#include "ModeleAerodynamique.h"
#include "Environnement.h"
#include "Constantes.h"
#include <cmath>
#include <limits>

CalculateurTrim::CalculateurTrim(ModeleAerodynamique& modele_aero, 
                                 const Environnement& environnement)
    : aero(modele_aero), env(environnement) {}

double CalculateurTrim::trouver_delta_profondeur(double pitch, double vitesse, 
                                                  double omega, double epsilon) {
    constexpr int n = 10000;
    constexpr double delta_p_min = -0.13;
    constexpr double delta_p_max = -0.01;
    constexpr double step = (delta_p_max - delta_p_min) / n;
    
    double best_delta_p = delta_p_min;
    double min_C_m = std::numeric_limits<double>::infinity();  //mouais voir niveau opti
    
    for (int i = 0; i <= n; ++i) {
        double delta_p = delta_p_min + i * step;
        aero.update_from_polar(pitch, delta_p, omega, vitesse);
        
        if (std::fabs(aero.C_m) < std::fabs(min_C_m)) {
            min_C_m = aero.C_m;
            best_delta_p = delta_p;
        }
        
        if (std::fabs(aero.C_m) < epsilon) break;
    }
    
    return best_delta_p;
}

double CalculateurTrim::trouver_alpha(double vitesse, double altitude, 
                                      double masse, double omega_pitch, double tol) {
    using namespace Physique;
    constexpr double alpha_min = 0.5 * DEG_TO_RAD;
    constexpr double alpha_max = 4.0 * DEG_TO_RAD;
    constexpr int n = 1000;
    constexpr double d_alpha = (alpha_max - alpha_min) / n;
    
    double alpha = alpha_min;
    double W = masse * g;
    double rho = env.calculer_rho(altitude);
    
    for (int i = 0; i < n; ++i) { // Liste de valeurs possibles pour delta_profondeur (par exemple, entre -1.0 et 1.0)
        double delta_p = trouver_delta_profondeur(alpha, vitesse, omega_pitch);
        aero.update_from_polar(alpha, delta_p, omega_pitch, vitesse);
        double L = aero.calculer_portance(vitesse, rho);
        
        if (std::fabs(L - W) < tol) break;
        
        alpha += (L < W) ? d_alpha : -d_alpha;
    }
    
    return alpha;
}
