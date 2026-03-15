#include "SauvetageAvion.h"
#include "Constantes.h"
#include "OptiSauvetageGeneral.h"
#include <cmath>
#include <algorithm>
#include <iostream>

#include <cassert> // debug

const char *SAUVETAGE_VERSION = "1.0";  /// ????

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

    ////////////////////////////////////////////
    // DEBUGGGGGGGG
    //if (status.en_descente_critique) {
    //std::cout << "SAUVETAGE DECLENCHE z=" << status.altitude
    //          << " vz=" << status.taux_descente
    //          << " pitch=" << status.assiette << std::endl;
    //}
        ////////////////////////////////////////////
        
    status.temps_depuis_manoeuvre = temps_courant - derniere_manoeuvre;
    status.cmd_profondeur_max = cmd_prof_max;
    status.cmd_thrust_max = cmd_thrust_max;
    
    return status;
}

// STRATEGY 0: Thrust reduction first, then profile (BEST)
// STRATEGY 1: Profile reduction first, then thrust
// STRATEGY 2: Both simultaneously
std::pair<double, double> SauvetageAvion::scenario_progressif(const EtatSauvetage& etat, OptiSauvetageGeneral::ParamsRescue* chromo){
    double t = etat.temps_depuis_manoeuvre;
    const double cmd_prof_min = etat.cmd_profondeur_max;
    const Rescue_Strategy STRATEGY = config.getEnum(STR_TO_RESCUE_STRATEGY, "rescue_strategy");
    
    const double cmd_thrust_min = etat.cmd_thrust_max * params_actifs.thrust_reduced_factor;
    const double cmd_prof_target = params_actifs.prof_reduced_factor * 0.5;
    
    double cmd_prof = cmd_prof_min;
    double cmd_thrust = etat.cmd_thrust_max;
    
    const double t_phase1 = params_actifs.phase_reduction_thrust;
    const double t_phase2 = t_phase1 + params_actifs.phase_reduction_prof;
    const double t_phase3 = t_phase2 + params_actifs.phase_control;
    
    switch (STRATEGY) {
        case Rescue_Strategy::THRUST_FIRST: {
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
        }

        case Rescue_Strategy::PROFILE_FIRST:{
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
        }

        case Rescue_Strategy::SIMULTANEOUS:{
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

        case Rescue_Strategy::GEN_FIND: {
            
            if (chromo==nullptr){
                throw std::runtime_error("Aucune chromosome donné comme support pour GEN_FIND");
            }
            // traduction de l'état en paramètre dicret compréhensible (on pourrait ajouter une fonction avec un struct etats_dicrets ca serait mieux mais la je veux juste avoir un truc)
            int z_env_discret = Discretisation(etat.altitude, SEUILS_Z);
            int vz_env_discret = Discretisation(etat.taux_descente, SEUILS_VZ);
            int vtot_env_discret = Discretisation(etat.vitesse, SEUILS_VTOT);
            int pitch_env_discret = Discretisation(etat.assiette, SEUILS_PITCH);
            int gamma_env_discret = Discretisation(etat.assiette - etat.alpha, SEUILS_GAMMA);

            bool known_strat = false ;

            for (int k_chromo = 0; k_chromo < int(chromo->vz_env.size()); k_chromo++) {   //int car sinon warning inutile avec int et unsigned int
                if (chromo->z_env[k_chromo] == z_env_discret          &&
                    chromo->vz_env[k_chromo] == vz_env_discret        &&
                    chromo->vtot_env[k_chromo] == vtot_env_discret    &&
                    chromo->pitch_env[k_chromo] == pitch_env_discret &&
                    chromo->gamma_env[k_chromo] == gamma_env_discret   ) 
                    {
                
                    cmd_thrust=etat.cmd_thrust_max*chromo->cmd_thrust_ratio_max[k_chromo];
                    cmd_prof=etat.cmd_profondeur_max*chromo->cmd_prof_ratio_max[k_chromo];
                    known_strat=true;

                    std::cout << "cmd_profondeur_max=" << etat.cmd_profondeur_max 
                                << " ratio=" << chromo->cmd_prof_ratio_max[k_chromo]
                                << " | cmd_thrust_max=" << etat.cmd_thrust_max 
                                << " ratio=" << chromo->cmd_thrust_ratio_max[k_chromo] << std::endl;
                    std::cout<<"cmd_thrust="<<cmd_thrust<<" | cmd_prof="<<cmd_prof<<std::endl;

                    break;
                }
            }
            // On a pas de stratégie donc on ajoute une au pif
            if (!known_strat) {
                double cmd_thrust_ratio_max=1.0*std::rand()/double(RAND_MAX);
                double cmd_prof_ratio_max=-1.0 +2.0*(std::rand()/double(RAND_MAX));  // précédent probleme avec ratio hors brone : depassement d'entier ici
                if ( (cmd_thrust_ratio_max<0 || cmd_thrust_ratio_max>1 ) || (cmd_prof_ratio_max<-1 || cmd_prof_ratio_max>1) ) {
                    throw std::runtime_error("La valeur de cmd tiré au pif si pas de known_strat est hors borne");
                    assert(cmd_prof_ratio_max >= -1.0 && cmd_prof_ratio_max <= 1.0);  // ← ici
                }
                chromo->push_gene(
                    z_env_discret,
                    vz_env_discret,
                    vtot_env_discret,
                    pitch_env_discret,
                    gamma_env_discret,
                    cmd_thrust_ratio_max,
                    cmd_prof_ratio_max
                );
                cmd_thrust=etat.cmd_thrust_max*cmd_thrust_ratio_max;
                cmd_prof=etat.cmd_profondeur_max*cmd_prof_ratio_max;

                std::cout << "cmd_profondeur_max=" << etat.cmd_profondeur_max 
                            << " ratio=" << cmd_prof_ratio_max
                            << " | cmd_thrust_max=" << etat.cmd_thrust_max 
                            << " ratio=" << cmd_thrust_ratio_max << std::endl;
            }
            std::cout<<"cmd_thrust="<<cmd_thrust<<" | cmd_prof="<<cmd_prof<<std::endl;
            break;
        }

        case Rescue_Strategy::GEN_GIVE: {
            if (chromo==nullptr){
                throw std::runtime_error("Aucune chromosome donné comme support pour GEN_GIVE");
            }
            throw std::runtime_error("Branche GEN_FIND non défini actuellement");
            break;
        }
    
    }
    
    cmd_prof = std::max(cmd_prof_min, cmd_prof);
    cmd_prof = std::max(-1.0, std::min(1.0, cmd_prof));
    cmd_thrust = std::max(0.0, std::min(1.0, cmd_thrust));
    std::cout<<"Correction cmd_thrust="<<cmd_thrust<<" | cmd_prof="<<cmd_prof<<std::endl;
    
    return {cmd_prof, cmd_thrust};
    
}
// c'est quoi déjà l'utilité de ça ???
std::pair<double, double> SauvetageAvion::appliquer_sauvetage(const EtatSauvetage& etat, OptiSauvetageGeneral::ParamsRescue* chromo){  // c'est quoi déjà l'utilité de ça ???
    return scenario_progressif(etat,chromo);
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

