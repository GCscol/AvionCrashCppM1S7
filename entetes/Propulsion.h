#ifndef PROPULSION_H
#define PROPULSION_H

class Propulsion {
private:
    double F0;           // Poussée par moteur (N)
    int n_moteurs;
    double expo_alt;
    
public:
    Propulsion(double poussee_nominale = 300000.0, int nb_moteurs = 2, 
               double expo_alt = 1.0);
    
    double calculer_poussee_max(double vitesse, double rho, double altitude) const;
};

#endif // PROPULSION_H
