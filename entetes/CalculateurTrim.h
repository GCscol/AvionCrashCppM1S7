#ifndef CALCULATEUR_TRIM_H
#define CALCULATEUR_TRIM_H

class ModeleAerodynamique;
class Environnement;

class CalculateurTrim {
private:
    ModeleAerodynamique& aero;
    const Environnement& env;
    
public:
    CalculateurTrim(ModeleAerodynamique& modele_aero, const Environnement& environnement);
    
    double trouver_delta_profondeur(double pitch, double vitesse, double omega, 
                                     double epsilon = 1e-6);
    
    double trouver_alpha(double vitesse, double altitude, double masse, 
                         double omega_pitch, double tol = 1e-6);
};

#endif // CALCULATEUR_TRIM_H
