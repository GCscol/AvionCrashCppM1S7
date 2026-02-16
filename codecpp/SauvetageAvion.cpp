#include "SauvetageAvion.h"
#include "Constantes.h"
#include <cmath>
#include <algorithm>

// Initialiser les constantes statiques
constexpr double SauvetageAvion::SEUIL_DESCENTE_CRITIQUE;
constexpr double SauvetageAvion::SEUIL_ALTITUDE_CRITIQUE;
constexpr double SauvetageAvion::SEUIL_PITCH_CRITIQUE;
constexpr double SauvetageAvion::PHASE_REDUCTION_THRUST;
constexpr double SauvetageAvion::PHASE_REDUCTION_PROF;
constexpr double SauvetageAvion::PHASE_CONTROL;
constexpr double SauvetageAvion::THRUST_REDUCED_FACTOR;
constexpr double SauvetageAvion::PROF_REDUCED_FACTOR;

SauvetageAvion::EtatSauvetage SauvetageAvion::evaluer_etat(
    const EtatCinematique& etat, 
    double temps_courant,
    double cmd_prof_max,
    double cmd_thrust_max,
    double derniere_manoeuvre)
{
    EtatSauvetage status;
    
    status.taux_descente = etat.vz;
    status.altitude = etat.z;
    status.vitesse = etat.get_vitesse_norme(); 
    status.assiette = etat.pitch;
    status.alpha = etat.get_alpha();
    
    // En descente critique si vz < seuil OU assiette trop negative
    // NOTE: Altitude trigger removed - rescue activates based on descent rate and pitch only
    bool descente = etat.vz < SEUIL_DESCENTE_CRITIQUE;
    bool nez_bas = etat.pitch < SEUIL_PITCH_CRITIQUE;
    status.en_descente_critique = (descente || nez_bas);  // No altitude restriction
    
    status.temps_depuis_manoeuvre = temps_courant - derniere_manoeuvre;
    
    // Garder en mémoire les maximales
    status.cmd_profondeur_max = cmd_prof_max;
    status.cmd_thrust_max = cmd_thrust_max;
    
    return status;
}

/**
 * Scénario PROGRESSIF: Test different command reduction orders
 * STRATEGY = 0: Thrust first, then profondeur (original)
 * STRATEGY = 1: Profondeur first, then thrust (alternative)
 * STRATEGY = 2: Both simultaneously
 */
std::pair<double, double> SauvetageAvion::scenario_progressif(const EtatSauvetage& etat)
{
    double t = etat.temps_depuis_manoeuvre;
    const double cmd_prof_min = etat.cmd_profondeur_max;
    const int STRATEGY = 0; // Strategy 0: Thrust FIRST, then profondeur (BEST)
    
    const double cmd_thrust_min = etat.cmd_thrust_max * THRUST_REDUCED_FACTOR;
    const double cmd_prof_target = PROF_REDUCED_FACTOR * 0.5; // Mild nose-up (currently -0.1)
    
    double cmd_prof = cmd_prof_min;
    double cmd_thrust = etat.cmd_thrust_max;
    
    const double t_phase1 = PHASE_REDUCTION_THRUST;
    const double t_phase2 = PHASE_REDUCTION_THRUST + PHASE_REDUCTION_PROF;
    const double t_phase3 = t_phase2 + PHASE_CONTROL;
    
    if (STRATEGY == 0) {
        // STRATEGY 0: Reduce thrust FIRST, then profile
        if (t < t_phase1) {
            // Phase 1: Reduce thrust
            double ratio = t / PHASE_REDUCTION_THRUST;
            cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
            cmd_prof = cmd_prof_min;
        } else if (t < t_phase2) {
            // Phase 2: Then reduce profondeur (nose-up)
            double ratio = (t - t_phase1) / PHASE_REDUCTION_PROF;
            cmd_thrust = cmd_thrust_min;
            cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
        } else if (t < t_phase3) {
            // Phase 3: Recover/stabilize
            double ratio = (t - t_phase2) / PHASE_CONTROL;
            cmd_thrust = cmd_thrust_min + (etat.cmd_thrust_max * 0.6 - cmd_thrust_min) * ratio;
            cmd_prof = cmd_prof_target;
        } else {
            cmd_thrust = etat.cmd_thrust_max * 0.6;
            cmd_prof = cmd_prof_target;
        }
    } 
    else if (STRATEGY == 1) {
        // STRATEGY 1: Reduce profondeur FIRST, then thrust
        if (t < t_phase1) {
            // Phase 1: Reduce profondeur (nose-up first)
            double ratio = t / PHASE_REDUCTION_THRUST;
            cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
            cmd_thrust = etat.cmd_thrust_max;
        } else if (t < t_phase2) {
            // Phase 2: Then reduce thrust
            double ratio = (t - t_phase1) / PHASE_REDUCTION_PROF;
            cmd_prof = cmd_prof_target;
            cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
        } else if (t < t_phase3) {
            // Phase 3: Recover/stabilize
            double ratio = (t - t_phase2) / PHASE_CONTROL;
            cmd_thrust = cmd_thrust_min + (etat.cmd_thrust_max * 0.6 - cmd_thrust_min) * ratio;
            cmd_prof = cmd_prof_target;
        } else {
            cmd_thrust = etat.cmd_thrust_max * 0.6;
            cmd_prof = cmd_prof_target;
        }
    }
    else if (STRATEGY == 2) {
        // STRATEGY 2: Reduce BOTH simultaneously
        if (t < t_phase1) {
            // Phase 1: Reduce both thrust and profondeur together
            double ratio = t / PHASE_REDUCTION_THRUST;
            cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
            cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
        } else if (t < t_phase2) {
            // Phase 2: Hold reduced commands
            cmd_thrust = cmd_thrust_min;
            cmd_prof = cmd_prof_target;
        } else if (t < t_phase3) {
            // Phase 3: Recover thrust
            double ratio = (t - t_phase2) / PHASE_CONTROL;
            cmd_thrust = cmd_thrust_min + (etat.cmd_thrust_max * 0.6 - cmd_thrust_min) * ratio;
            cmd_prof = cmd_prof_target;
        } else {
            cmd_thrust = etat.cmd_thrust_max * 0.6;
            cmd_prof = cmd_prof_target;
        }
    }
    
    // Clamp to limits
    cmd_prof = std::max(cmd_prof_min, cmd_prof);
    cmd_prof = std::max(-1.0, std::min(1.0, cmd_prof));
    cmd_thrust = std::max(0.0, std::min(1.0, cmd_thrust));
    
    return {cmd_prof, cmd_thrust};
}

std::pair<double, double> SauvetageAvion::appliquer_sauvetage(const EtatSauvetage& etat)
{
    return scenario_progressif(etat);
}

bool SauvetageAvion::verifier_succes_sauvetage(
    const EtatCinematique& etat_courant,
    const EtatCinematique& /* etat_initial_sauvetage */,
    double /* temps_ecoule */,
    double temps_vz_positif)
{
    // Succes si:
    // 1) vz > 0 pendant au moins 2 s
    // 2) alpha < 14 deg
    // 3) V dans [120, 350] m/s

    double alpha_deg = etat_courant.get_alpha() * Physique::RAD_TO_DEG;
    double speed = etat_courant.get_vitesse_norme();

    bool vz_ok = temps_vz_positif >= 2.0;
    bool alpha_ok = alpha_deg < 14.0;
    bool speed_ok = speed >= 120.0 && speed <= 350.0;

    return vz_ok && alpha_ok && speed_ok;
}

