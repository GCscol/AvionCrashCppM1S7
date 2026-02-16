#ifndef AVION_H
#define AVION_H

#include "ModeleAerodynamique.h"
#include "ModeleLineaire.h"
#include "ModeleHysteresis.h"
#include "ProprietesInertie.h"
#include "Propulsion.h"
#include "SystemeControle.h"
#include "Environnement.h"
#include "CalculateurTrim.h"
#include "EtatCinematique.h"
#include "ForcesAerodynamiques.h"
#include "Integration.h"

class Avion {
private:
    // Composition des systèmes
    ModeleAerodynamique* aero;
    ProprietesInertie inertie;
    Propulsion propulsion;
    SystemeControle controle;
    Environnement environnement;
    double altitude_croisiere;
    CalculateurTrim* trim_calculator;

    EtatCinematique etat;
    ForcesAerodynamiques forces;
    
public:
    Avion(double surface, double corde, double masse, bool useHysteresis = false);
    ~Avion();
    
    const Environnement& get_env() const;
    ModeleAerodynamique& get_aero();
    const ModeleAerodynamique& get_aero() const;
    
    double get_masse() const;
    void set_masse(double m);
    
    double& z_ref();  
    double get_altitude() const;
    
    EtatCinematique& get_etat();
    const EtatCinematique& get_etat() const;
    
    SystemeControle& get_controle();
    
    ForcesAerodynamiques& get_forces();
    const ForcesAerodynamiques& get_forces() const;
    
    double get_pitch() const;
    double get_omega_pitch() const;
    double get_vitesse_x() const;
    double get_vitesse_y() const;
    double get_vitesse_z() const;
    double get_x() const;
    double get_y() const;
    double get_roll() const;
    double get_yaw() const;
    
    double get_portance() const;
    double get_trainee() const;
    double get_traction() const;
    double get_M_pitch() const;
    double get_Fx() const;
    double get_Fy() const;
    double get_Fz() const;

    double get_I_pitch() const;
    
    double get_cmd_profondeur() const;
    
    void initialiser();
    
    double calculer_poussee_max(double vitesse, double rho, double altitude) const;
    
    void calculer_forces();
    void calculer_forces(double& L, double& D, double& T, double& W);
    

    double trouver_delta_profondeur(double vitesse, double omega);
    double trouver_alpha(double vitesse);
    
        // Intégration (Euler)
    void mettre_a_jour_etat(double dt);
};

#endif // AVION_H
