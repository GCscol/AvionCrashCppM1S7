#ifndef CALCULATEUR_TRIM_H
#define CALCULATEUR_TRIM_H

#include <utility>

class ModeleAerodynamique;
class Environnement;

class CalculateurTrim {
private:
    ModeleAerodynamique& aero;
    const Environnement& env;
    
public:
    CalculateurTrim(ModeleAerodynamique& modele_aero, const Environnement& environnement);
    
    double trouver_delta_profondeur(double pitch, double vitesse, double altitude, double omega,
                                     double epsilon = 1e-6,
                                     double delta_p_min = -0.20,
                                     double delta_p_max = 0.05);
    
    double trouver_alpha(double vitesse, double altitude, double masse, 
                         double omega_pitch, double tol = 1e-6,
                         double alpha_min = 0.0,  // -1.0 deg in rad
                         double alpha_max = 0.261799);  // 15.0 deg in rad
    
    // Return (alpha, delta_p) pair simultaneously
    std::pair<double, double> calculer_trim_complet(double vitesse, double altitude, 
                                                     double masse, double omega_pitch,
                                                     double alpha_min = -0.017453,
                                                     double alpha_max = 0.261799,
                                                     double delta_p_min = -0.40,
                                                     double delta_p_max = 0.05);
};

#endif // CALCULATEUR_TRIM_H
