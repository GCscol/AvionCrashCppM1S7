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
                                     double epsilon = 1e-6);
    
    double trouver_alpha(double vitesse, double altitude, double masse, 
                         double omega_pitch, double tol = 1e-6);
    
    // Return (alpha, delta_p) pair simultaneously
    std::pair<double, double> calculer_trim_complet(double vitesse, double altitude, 
                                                     double masse, double omega_pitch);
};

#endif // CALCULATEUR_TRIM_H
