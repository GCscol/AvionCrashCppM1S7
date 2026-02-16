#ifndef MODELE_AERODYNAMIQUE_H
#define MODELE_AERODYNAMIQUE_H

class ModeleAerodynamique {
protected:
    double S;                    // Surface alaire de l'avion (en m²)
    double l;                    // Corde moyenne
    double delta_profondeur;     // Choisi à alpha=1.5 deg  , position de trim
    
public:
    double C_L;   // Coefficient de portance
    double C_D;   // Coefficient de traînée
    double C_m;   // Coefficient de moment de tangage
    double omega_pitch;
    
    ModeleAerodynamique(double surface, double corde, double delta_prof_init);
    virtual ~ModeleAerodynamique() = default;
    

    virtual void update_from_polar(double alpha_rad, double delta_p, 
                                   double omega, double vitesse) = 0;
    

    double get_surface() const;
    double get_corde() const;
    double get_delta_profondeur() const;
    void set_delta_profondeur(double val);
    

    double calculer_portance(double vitesse, double rho) const;
    double calculer_trainee(double vitesse, double rho) const;
    double calculer_moment_pitch(double vitesse, double rho) const;
};

#endif // MODELE_AERODYNAMIQUE_H
