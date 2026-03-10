#include "SauvetageAvion.h"
#include "Constantes.h"
#include <cmath>
#include <algorithm>

const char *SAUVETAGE_VERSION = "1.0";

SauvetageAvion::Parametres SauvetageAvion::params_actifs{};
double SauvetageAvion::seuil_descente_critique_actif = SauvetageAvion::SEUIL_DESCENTE_CRITIQUE_DEFAUT;
double SauvetageAvion::seuil_pitch_critique_actif = SauvetageAvion::SEUIL_PITCH_CRITIQUE_DEFAUT;

void SauvetageAvion::set_parametres(const Parametres& params)
{
    params_actifs = params;
}

SauvetageAvion::Parametres SauvetageAvion::get_parametres()
{
    return params_actifs;
}

void SauvetageAvion::reset_parametres_defaut()
{
    params_actifs = Parametres{};
}

void SauvetageAvion::set_seuils_critiques(double seuil_descente, double seuil_pitch)
{
    seuil_descente_critique_actif = seuil_descente;
    seuil_pitch_critique_actif = seuil_pitch;
}

std::pair<double, double> SauvetageAvion::get_seuils_critiques()
{
    return {seuil_descente_critique_actif, seuil_pitch_critique_actif};
}

void SauvetageAvion::reset_seuils_critiques_defaut()
{
    seuil_descente_critique_actif = SEUIL_DESCENTE_CRITIQUE_DEFAUT;
    seuil_pitch_critique_actif = SEUIL_PITCH_CRITIQUE_DEFAUT;
}


SauvetageAvion::EtatSauvetage SauvetageAvion::evaluer_etat(
    const EtatCinematique& etat, 
    double temps_courant,
    double cmd_prof_max,
    double cmd_thrust_max,
    double derniere_manoeuvre,
    double seuil_altitude_critique)
{
    EtatSauvetage status;
    
    status.taux_descente = etat.vz;
    status.altitude = etat.z;
    status.vitesse = etat.get_vitesse_norme(); 
    status.assiette = etat.pitch;
    status.alpha = etat.get_alpha();
    
    // Critical if BOTH conditions are met:
    // 1. Altitude below threshold
    // 2. Descent rate below threshold OR pitch too negative
    bool altitude_critique = etat.z < seuil_altitude_critique;
    bool descente = etat.vz < seuil_descente_critique_actif;
    bool nez_bas = etat.pitch < seuil_pitch_critique_actif;
    status.en_descente_critique = altitude_critique && (descente || nez_bas);
    
    status.temps_depuis_manoeuvre = temps_courant - derniere_manoeuvre;
    status.cmd_profondeur_max = cmd_prof_max;
    status.cmd_thrust_max = cmd_thrust_max;
    
    return status;
}

// STRATEGY 0: Thrust reduction first, then profile (BEST)
// STRATEGY 1: Profile reduction first, then thrust
// STRATEGY 2: Both simultaneously
std::pair<double, double> SauvetageAvion::scenario_progressif(const EtatSauvetage& etat){
    double t = etat.temps_depuis_manoeuvre;
    const double cmd_prof_min = etat.cmd_profondeur_max;
    const Rescue_Strategy STRATEGY = config.getEnum(STR_TO_RESCUE_STRATEGY, "rescue_strategy");
    
    const double cmd_thrust_min = etat.cmd_thrust_max * params_actifs.thrust_reduced_factor;
    const double cmd_prof_target = params_actifs.prof_reduced_factor * 0.5;
    
    double cmd_prof = cmd_prof_min;
    double cmd_thrust = etat.cmd_thrust_max;
    
    const double t_phase1 = params_actifs.phase_reduction_thrust;
    const double t_phase2 = params_actifs.phase_reduction_thrust + params_actifs.phase_reduction_prof;
    const double t_phase3 = t_phase2 + params_actifs.phase_control;
    
    switch (STRATEGY) {
        case Rescue_Strategy::THRUST_FIRST:
            if (t < t_phase1) {
                // Phase 1: Reduce thrust only
                double ratio = t / params_actifs.phase_reduction_thrust;
                cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
                cmd_prof = cmd_prof_min;
            } else if (t < t_phase2) {
                // Phase 2: Reduce pitch (nose-up)
                double ratio = (t - t_phase1) / params_actifs.phase_reduction_prof;
                cmd_thrust = cmd_thrust_min;
                cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
            } else if (t < t_phase3) {
                // Phase 3: Stabilize
                double ratio = (t - t_phase2) / params_actifs.phase_control;
                cmd_thrust = cmd_thrust_min + (etat.cmd_thrust_max * params_actifs.stabilization_thrust_factor - cmd_thrust_min) * ratio;
                cmd_prof = cmd_prof_target;
            } else {
                cmd_thrust = etat.cmd_thrust_max * params_actifs.stabilization_thrust_factor;
                cmd_prof = cmd_prof_target;
            }
            break;
         
        case Rescue_Strategy::PROFILE_FIRST:
            if (t < t_phase1) {
                // Phase 1: Reduce pitch first
                double ratio = t / params_actifs.phase_reduction_thrust;
                cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
                cmd_thrust = etat.cmd_thrust_max;
            } else if (t < t_phase2) {
                // Phase 2: Reduce thrust
                double ratio = (t - t_phase1) / params_actifs.phase_reduction_prof;
                cmd_prof = cmd_prof_target;
                cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
            } else if (t < t_phase3) {
                // Phase 3: Stabilize
                double ratio = (t - t_phase2) / params_actifs.phase_control;
                cmd_thrust = cmd_thrust_min + (etat.cmd_thrust_max * params_actifs.stabilization_thrust_factor - cmd_thrust_min) * ratio;
                cmd_prof = cmd_prof_target;
            } else {
                cmd_thrust = etat.cmd_thrust_max * params_actifs.stabilization_thrust_factor;
                cmd_prof = cmd_prof_target;
            }
            break;
        
        case Rescue_Strategy::SIMULTANEOUS:
        if (t < t_phase1) {
            // Phase 1: Reduce both simultaneously
            double ratio = t / params_actifs.phase_reduction_thrust;
            cmd_thrust = etat.cmd_thrust_max + (cmd_thrust_min - etat.cmd_thrust_max) * ratio;
            cmd_prof = cmd_prof_min + (cmd_prof_target - cmd_prof_min) * ratio;
        } else if (t < t_phase2) {
            // Phase 2: Hold reduced state
            cmd_thrust = cmd_thrust_min;
            cmd_prof = cmd_prof_target;
        } else if (t < t_phase3) {
            // Phase 3: Recover thrust
            double ratio = (t - t_phase2) / params_actifs.phase_control;
            cmd_thrust = cmd_thrust_min + (etat.cmd_thrust_max * params_actifs.stabilization_thrust_factor - cmd_thrust_min) * ratio;
            cmd_prof = cmd_prof_target;
        } else {
            cmd_thrust = etat.cmd_thrust_max * params_actifs.stabilization_thrust_factor;
            cmd_prof = cmd_prof_target;
        }
        break;
    
    }
    cmd_prof = std::max(cmd_prof_min, cmd_prof);
    cmd_prof = std::max(-1.0, std::min(1.0, cmd_prof));
    cmd_thrust = std::max(0.0, std::min(1.0, cmd_thrust));
    
    return {cmd_prof, cmd_thrust};
    
}

std::pair<double, double> SauvetageAvion::appliquer_sauvetage(const EtatSauvetage& etat){
    return scenario_progressif(etat);
}

bool SauvetageAvion::verifier_succes_sauvetage( //Chelou
    const EtatCinematique& etat_courant,
    const EtatCinematique& /* etat_initial_sauvetage */,
    double /* temps_ecoule */,
    double temps_vz_positif)
    {
    // Success if: vz > 0 for >= 2s, alpha < 14°, speed in [120, 350] m/s
    double alpha_deg = etat_courant.get_alpha() * Math::RAD_TO_DEG;
    double speed = etat_courant.get_vitesse_norme();

    bool vz_ok = temps_vz_positif >= 2.0;
    bool alpha_ok = alpha_deg < 14.0;
    bool speed_ok = speed >= 120.0 && speed <= 350.0;

    return vz_ok && alpha_ok && speed_ok;
}

