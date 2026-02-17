#include "SauvetageAvion.h"
#include "Constantes.h"
#include <cmath>
#include <algorithm>

const char *SAUVETAGE_VERSION = "1.0";

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
    
    // Critical if descent rate below threshold OR pitch too negative
    bool descente = etat.vz < SEUIL_DESCENTE_CRITIQUE;
    bool nez_bas = etat.pitch < SEUIL_PITCH_CRITIQUE;
    status.en_descente_critique = (descente || nez_bas);
    
    status.temps_depuis_manoeuvre = temps_courant - derniere_manoeuvre;
    status.cmd_profondeur_max = cmd_prof_max;
    status.cmd_thrust_max = cmd_thrust_max;
    
    return status;
}

// STRATEGY 0: Thrust reduction first, then profile (BEST)
// STRATEGY 1: Profile reduction first, then thrust
// STRATEGY 2: Both simultaneously
std::pair<double, double> SauvetageAvion::scenario_progressif(const EtatSauvetage& etat)
{
    double t = etat.temps_depuis_manoeuvre;
    const double cmd_prof_min = etat.cmd_profondeur_max;
    const int STRATEGY = 0; // Optimal strategy: thrust-first approach
    
    const double cmd_thrust_min = etat.cmd_thrust_max * THRUST_REDUCED_FACTOR;
    const double cmd_prof_target = PROF_REDUCED_FACTOR * 0.5;
    
    double cmd_prof = cmd_prof_min;
    double cmd_thrust = etat.cmd_thrust_max;
    
    const double t_phase1 = PHASE_REDUCTION_THRUST;
    const double t_phase2 = PHASE_REDUCTION_THRUST + PHASE_REDUCTION_PROF;
    const double t_phase3 = t_phase2 + PHASE_CONTROL;
    
    if (STRATEGY == 0) {
        if (t < t_phase1) {
            // Phase 1: Reduce thrust only
            double ratio = t / PHASE_REDUCTION_THRUST;
            cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
            cmd_prof = cmd_prof_min;
        } else if (t < t_phase2) {
            // Phase 2: Reduce pitch (nose-up)
            double ratio = (t - t_phase1) / PHASE_REDUCTION_PROF;
            cmd_thrust = cmd_thrust_min;
            cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
        } else if (t < t_phase3) {
            // Phase 3: Stabilize
            double ratio = (t - t_phase2) / PHASE_CONTROL;
            cmd_thrust = cmd_thrust_min + (etat.cmd_thrust_max * 0.6 - cmd_thrust_min) * ratio;
            cmd_prof = cmd_prof_target;
        } else {
            cmd_thrust = etat.cmd_thrust_max * 0.6;
            cmd_prof = cmd_prof_target;
        }
    } 
    else if (STRATEGY == 1) {
        if (t < t_phase1) {
            // Phase 1: Reduce pitch first
            double ratio = t / PHASE_REDUCTION_THRUST;
            cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
            cmd_thrust = etat.cmd_thrust_max;
        } else if (t < t_phase2) {
            // Phase 2: Reduce thrust
            double ratio = (t - t_phase1) / PHASE_REDUCTION_PROF;
            cmd_prof = cmd_prof_target;
            cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
        } else if (t < t_phase3) {
            // Phase 3: Stabilize
            double ratio = (t - t_phase2) / PHASE_CONTROL;
            cmd_thrust = cmd_thrust_min + (etat.cmd_thrust_max * 0.6 - cmd_thrust_min) * ratio;
            cmd_prof = cmd_prof_target;
        } else {
            cmd_thrust = etat.cmd_thrust_max * 0.6;
            cmd_prof = cmd_prof_target;
        }
    }
    else if (STRATEGY == 2) {
        if (t < t_phase1) {
            // Phase 1: Reduce both simultaneously
            double ratio = t / PHASE_REDUCTION_THRUST;
            cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
            cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
        } else if (t < t_phase2) {
            // Phase 2: Hold reduced state
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
    // Success if: vz > 0 for >= 2s, alpha < 14°, speed in [120, 350] m/s
    double alpha_deg = etat_courant.get_alpha() * Physique::RAD_TO_DEG;
    double speed = etat_courant.get_vitesse_norme();

    bool vz_ok = temps_vz_positif >= 2.0;
    bool alpha_ok = alpha_deg < 14.0;
    bool speed_ok = speed >= 120.0 && speed <= 350.0;

    return vz_ok && alpha_ok && speed_ok;
}

