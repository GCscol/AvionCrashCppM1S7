#ifndef MODELE_LINEAIRE_H
#define MODELE_LINEAIRE_H

#include "ModeleAerodynamique.h"

class ModeleLineaire : public ModeleAerodynamique {
public:
    ModeleLineaire(double surface, double corde, double delta_prof_init = 0.0);
    
    void update_from_polar(double alpha_rad, double delta_p, 
                          double omega, double vitesse) override;
};

#endif // MODELE_LINEAIRE_H
