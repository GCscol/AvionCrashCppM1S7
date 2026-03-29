#include "Avion.h"
#include "ModeleLineaire.h"
#include "ModeleHysteresis.h"
#include "Integration.h"
#include "Constantes.h"
#include <cmath>

Avion::Avion(double surface, double corde, double masse, bool useHysteresis)
    : inertie(masse),
      propulsion(),
      environnement(),
      altitude_croisiere(11000.0),
      trim_calculator(nullptr)
{
    if (useHysteresis) {
        aero = new ModeleHysteresis(surface, corde, 0.0);
    } else {
        aero = new ModeleLineaire(surface, corde, 0.0);
    }
    trim_calculator = new CalculateurTrim(*aero, environnement);
}

Avion::~Avion() {
    delete aero;
    if (trim_calculator) delete trim_calculator;
}

const Environnement& Avion::get_env() const { return environnement; }

ModeleAerodynamique& Avion::get_aero() { return *aero; }

const ModeleAerodynamique& Avion::get_aero() const { return *aero; }

double Avion::get_masse() const {
    return inertie.get_masse();
}

void Avion::set_masse(double m) {
    inertie.set_masse(m);
}

double& Avion::z_ref() {
    return etat.z;
}

double Avion::get_altitude() const {
    return etat.z;
}

EtatCinematique& Avion::get_etat() {  
    return etat;
}

const EtatCinematique& Avion::get_etat() const {
    return etat;
}

SystemeControle& Avion::get_controle() {
    return controle;
}

ForcesAerodynamiques& Avion::get_forces() {
    return forces;
}

const ForcesAerodynamiques& Avion::get_forces() const {
    return forces;
}

double Avion::get_pitch() const { return etat.pitch; }
double Avion::get_omega_pitch() const { return etat.omega_pitch; }
double Avion::get_vitesse_x() const { return etat.vx; }
double Avion::get_vitesse_y() const { return etat.vy; }
double Avion::get_vitesse_z() const { return etat.vz; }
double Avion::get_x() const { return etat.x; }
double Avion::get_y() const { return etat.y; }
double Avion::get_roll() const { return etat.roll; }
double Avion::get_yaw() const { return etat.yaw; }

double Avion::get_portance() const { return forces.portance; }
double Avion::get_trainee() const { return forces.trainee; }
double Avion::get_traction() const { return forces.traction; }
double Avion::get_M_pitch() const { return forces.M_pitch; }
double Avion::get_Fx() const { return forces.Fx; }
double Avion::get_Fy() const { return forces.Fy; }
double Avion::get_Fz() const { return forces.Fz; }

double Avion::get_I_pitch() const { return inertie.get_I_pitch(); }

double Avion::get_cmd_profondeur() const {
    return controle.get_cmd_profondeur();
}
void Avion::initialiser(double vx, double z) {
    etat.x = 0.0; etat.y = 0.0; etat.z = z;
    etat.vx = vx; etat.vy = 0.0; etat.vz = 0.0;
    etat.roll = 0.0; 
    etat.pitch = 1.0 / Math::RAD_TO_DEG;
    etat.yaw = 0.0;
    etat.omega_pitch = 0.0;
}

    // Calcul de poussée maximale
double Avion::calculer_poussee_max(double vitesse, double rho, double altitude) const {
    return propulsion.calculer_poussee_max(vitesse, rho, altitude);
}

    // Calcul des forces
void Avion::calculer_forces() {
    double vitesse = etat.get_vitesse_norme();
    double rho = environnement.calculer_rho(etat.z);
    
    forces.portance = aero->calculer_portance(vitesse, rho);
    forces.trainee = aero->calculer_trainee(vitesse, rho);
    forces.poids = inertie.calculer_poids();
    
    double traction_max = propulsion.calculer_poussee_max(vitesse, rho, etat.z);
    forces.traction = controle.get_cmd_thrust() * traction_max;
}

void Avion::calculer_forces(double& L, double& D, double& T, double& W) {
    calculer_forces();
    L = forces.portance;
    D = forces.trainee;
    T = forces.traction;
    W = forces.poids;
}

    // Fonctions de trim
double Avion::trouver_delta_profondeur(double vitesse, double omega) {
    return trim_calculator->trouver_delta_profondeur(etat.pitch, vitesse, etat.z, omega);
}

double Avion::trouver_alpha(double vitesse) {
    return trim_calculator->trouver_alpha(vitesse, etat.z, 
                                         inertie.get_masse(), 
                                         etat.omega_pitch);
}

std::pair<double, double> Avion::calculer_trim_complet(double vitesse) {
    return trim_calculator->calculer_trim_complet(vitesse, etat.z, 
                                                   inertie.get_masse(), 
                                                   etat.omega_pitch);
}


void Avion::mettre_a_jour_etat(double dt) {

    switch (config.getEnum(STR_TO_METHODE, "methode_integration")) {
        case Methode_Integration::EULER:
            Integration::Euler(*this, dt);
            break;

        case Methode_Integration::RK4:
            Integration::RK4(*this, dt);
            break;
    }

}
