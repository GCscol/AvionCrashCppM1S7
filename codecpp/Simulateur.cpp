#include "Simulateur.h"
#include "Avion.h"
#include "Constantes.h"
#include "SauvetageAvion.h"
#include "OptiSauvetageGeneral.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

using namespace std;

namespace {
bool sim_logs_enabled() {
    try {
        return !config.getBool("quiet_optimizer_logs");
    } catch (...) {
        return true;
    }
}
}

double Simulateur::get_dernier_temps_recuperation() const {
    return dernier_temps_recuperation;
}

double Simulateur::get_derniere_altitude_recuperation() const {
    return derniere_altitude_recuperation;
}


Simulateur::Simulateur(Avion& av, double pas_temps, double duree,
                                             const string& fichier,
                                             double cmd_profondeur,
                                             double cmd_thrust,
                                             double cmd_start,
                                             double cmd_end,
                                             bool enable_rescue_system,
                                             double altitude_critique)
        : avion(av), dt(pas_temps), temps_total(duree), fichier_sortie(fichier),
            test_cmd_profondeur(cmd_profondeur), test_cmd_thrust(cmd_thrust),
            test_cmd_start(cmd_start), test_cmd_end(cmd_end),
            enable_rescue(enable_rescue_system), temps_debut_sauvetage(-1e6),
            seuil_altitude_critique(altitude_critique){}

double Simulateur::executer(OptiSauvetageGeneral::ParamsRescue* chromo) {  // empty by defaut
    using namespace Math;
    
    derniere_altitude_recuperation = 0.0;
    dernier_temps_recuperation     = -1e6;  // neg pour détecter si crash avant sauvegarde activé
    temps_debut_sauvetage          = -3e6;
    
    
    const int steps = static_cast<int>(temps_total / dt);
    
    std::ofstream csv(fichier_sortie);
    if (!csv.is_open()) {
        std::cerr << "Impossible d'ouvrir " << fichier_sortie << " en écriture\n";
        return std::numeric_limits<double>::quiet_NaN();
    }
    
        // Initialisation du trim complet (alpha, delta_profondeur ET cmd_thrust)
    double speed = avion.get_vitesse_x();
    
    // Use complete trim convergence method
    std::pair<double, double> trim_result = avion.calculer_trim_complet(speed);
    double alpha_trim = trim_result.first;
    double delta_trim = trim_result.second;
    
    avion.get_etat().pitch = alpha_trim;
    avion.get_aero().set_delta_profondeur(delta_trim);
    
    double rho = avion.get_env().calculer_rho(avion.get_altitude());
    avion.get_aero().update_from_polar(alpha_trim, delta_trim, 0.0, speed);
    double trainee_trim = avion.get_aero().calculer_trainee(speed, rho);
    double traction_max = avion.calculer_poussee_max(speed, rho, avion.get_altitude());
    double cmd_thrust_trim = std::numeric_limits<double>::quiet_NaN();
    
    if (traction_max > 0.0) {
        cmd_thrust_trim = trainee_trim / traction_max;
        // Clamp to [0, 1]
        cmd_thrust_trim = std::max(0.0, std::min(1.0, cmd_thrust_trim));
        avion.get_controle().set_commande_thrust(cmd_thrust_trim);
    }
    
    csv << "time,"
        << "x,y,z,vx,vy,vz,"
        << "roll,pitch,yaw,"
        << "M_pitch,M_thrust,"
        << "Fx,Fy,Fz,portance,trainee,traction,"
        << "Cl,Cd,Cm,"
        << "speed,AoA_deg,cmd_profondeur,cmd_thrust,alpha,delta_profondeur,n_factor\n";
    
        // Determine if this is the control_test mode (special command sequence)

        // Boucle de simulation
    double crash_time = std::numeric_limits<double>::quiet_NaN();
    bool rescue_activated = false;
    bool rescue_successful = false;
    EtatCinematique etat_avant_sauvetage;
    int rescue_activation_count = 0;
    bool rescue_cooldown_active = false;
    bool rescue_hold_active = false;
    bool rescue_completed = false;
    double rescue_cooldown_end = 0.0;
    const double rescue_cooldown_sec = 25.0;
    double rescue_vz_positive_time = 0.0;

    // Stall speed monitoring (minimum lift-sustaining speed)
    const double surface = avion.get_aero().get_surface();
    const double poids = avion.get_masse() * config.getDouble("g");
    const double alpha_stall_rad = 15.0 * DEG_TO_RAD;
    const double CL_max = 5.0 * (alpha_stall_rad - (-0.035)) + 0.44 * (-0.13);
    bool etait_sous_vmin = false;
    
    for (int i = 0; i < steps; ++i) {
        double t = (i + 1) * dt;
        
        avion.mettre_a_jour_etat(dt);
        
        double speed = avion.get_etat().get_vitesse_norme();
        double alpha = avion.get_etat().get_alpha();
        double AoA_deg = alpha * RAD_TO_DEG;
        
        if (!std::isfinite(speed) || speed > 5000.0) {  // punir situation non physique
            derniere_altitude_recuperation = -1e9;
            dernier_temps_recuperation = -2.0;
            break;
        } 

        // Compute current minimum lift-sustaining speed and detect crossing below threshold
        double rho_now = avion.get_env().calculer_rho(avion.get_altitude());
        double v_min_sustentation = std::sqrt(2.0 * poids / (rho_now * surface * CL_max));
        bool sous_vmin = (speed < v_min_sustentation);
        if (sous_vmin && !etait_sous_vmin) {
            if (sim_logs_enabled()) {
                std::cout << "[T=" << t << "s] : vitesse sous la sustentation minimale! "
                          << std::endl;
            }
        }
        etait_sous_vmin = sous_vmin;
        
        if (rescue_cooldown_active) {
            avion.get_controle().set_commande_profondeur(0.0);
            if (!std::isnan(cmd_thrust_trim)) {
                avion.get_controle().set_commande_thrust(cmd_thrust_trim);
            } else {
                avion.get_controle().set_commande_thrust(0.0);
            }

            // Enregistrer aussi l'état pendant la période de cooldown/stabilisation
            {
                double n_factor_local = avion.get_portance() / (avion.get_masse() * config.getDouble("g"));
                csv << t << ',' 
                    << avion.get_x() << ',' << avion.get_y() << ',' << avion.get_altitude() << ','
                    << avion.get_vitesse_x() << ',' << avion.get_vitesse_y() << ',' << avion.get_vitesse_z() << ','
                    << avion.get_roll() << ',' << avion.get_pitch() << ',' << avion.get_yaw() << ','
                    << avion.get_M_pitch() << ',' << avion.get_forces().M_thrust << ','
                    << avion.get_Fx() << ',' << avion.get_Fy() << ',' << avion.get_Fz() << ','
                    << avion.get_portance() << ',' << avion.get_trainee() << ',' << avion.get_traction() << ','
                    << avion.get_aero().C_L << ',' << avion.get_aero().C_D << ',' << avion.get_aero().C_m << ','
                    << avion.get_etat().get_vitesse_norme() << ',' << (avion.get_etat().get_alpha() * RAD_TO_DEG) << ','
                    << avion.get_cmd_profondeur() << ',' << avion.get_controle().get_cmd_thrust() << ','
                    << avion.get_etat().get_alpha() << ',' << avion.get_aero().get_delta_profondeur() << ','
                    << n_factor_local << '\n';
            }

            if (t >= rescue_cooldown_end) {
                if (sim_logs_enabled()) {
                    std::cout << "[T=" << t << "s] Arret simulation: profil stabilise." << std::endl;
                }
                break;
            }
            continue;
        }

        if (rescue_hold_active) {
            // Maintenir un profil stabilise apres sauvetage
            avion.get_controle().set_commande_profondeur(0.0);
            if (!std::isnan(cmd_thrust_trim)) {
                avion.get_controle().set_commande_thrust(cmd_thrust_trim);
            } else {
                avion.get_controle().set_commande_thrust(0.0);
            }
            // On laisse la simulation continuer avec ce profil
        }

        // Rescue system management
        if (enable_rescue && !rescue_completed) {
            // Evaluate aircraft state with max values and altitude threshold
            SauvetageAvion::EtatSauvetage etat_sauvetage = SauvetageAvion::evaluer_etat(
                avion.get_etat(), t, test_cmd_profondeur, test_cmd_thrust, 
                temps_debut_sauvetage, seuil_altitude_critique);
            
            // Si en descente critique ET pas encore en sauvetage
            if (etat_sauvetage.en_descente_critique && !rescue_activated) {
                rescue_activated = true;
                rescue_successful = false;
                rescue_vz_positive_time = 0.0;
                rescue_activation_count++;
                temps_debut_sauvetage = t;
                etat_avant_sauvetage = avion.get_etat();

                if (sim_logs_enabled()) {
                    std::cout << "\n========== ACTIVATION SAUVETAGE AUTOMATIQUE (#" << rescue_activation_count << ") ==========" << std::endl;
                    std::cout << "[T=" << t << "s] Situation critique detectee:" << std::endl;
                    std::cout << "  - Altitude: " << avion.get_altitude() << " m" << std::endl;
                    std::cout << "  - Taux descente: " << avion.get_vitesse_z() << " m/s" << std::endl;
                    std::cout << "  - Assiette: " << (avion.get_pitch() * RAD_TO_DEG) << " deg" << std::endl;
                    std::cout << "  - Scenario choisi: PROGRESSIF (douce)" << std::endl;
                }
            }
            
            // Applique les commandes de sauvetage si actif
            if (rescue_activated) {
                if (avion.get_vitesse_z() > 0.0) {
                    rescue_vz_positive_time += dt;
                } else {
                    rescue_vz_positive_time = 0.0;
                }

                // Réévalue l'état à chaque pas avec le temps depuis le début de la manoeuvre
                etat_sauvetage = SauvetageAvion::evaluer_etat(
                    avion.get_etat(), t, test_cmd_profondeur, test_cmd_thrust, 
                    temps_debut_sauvetage, seuil_altitude_critique);
                
                std::pair<double, double> cmds = SauvetageAvion::appliquer_sauvetage(etat_sauvetage, chromo);
                
                avion.get_controle().set_commande_profondeur(cmds.first);
                avion.get_controle().set_commande_thrust(cmds.second);

                // Vérifie si le sauvetage a réussi
                double temps_sauvetage = t - temps_debut_sauvetage;
                if (temps_sauvetage >= 2.0) {   // Remplacer le 2.0 pour etre constant avec la condition de test du sauvetage
                    rescue_successful = SauvetageAvion::verifier_succes_sauvetage(
                        avion.get_etat(), etat_avant_sauvetage, temps_sauvetage, rescue_vz_positive_time);
                    
                    if (rescue_successful) {
                        if (sim_logs_enabled()) {
                            std::cout << "[T=" << t << "s] SAUVETAGE REUSSI" << std::endl;
                            std::cout << "  - Altitude retrouvée: " << avion.get_altitude() << " m" << std::endl;
                            std::cout << "  - Vitesse vertical: " << avion.get_vitesse_z() << " m/s" << std::endl;
                            std::cout << "  - Vitesse: " << avion.get_etat().get_vitesse_norme() << " m/s" << std::endl;
                            std::cout << "  - Angle d'attaque:" << avion.get_etat().get_alpha() * RAD_TO_DEG << "deg." << std::endl;
                        }
                        rescue_activated = false;  // Fin du sauvetage
                        rescue_cooldown_active = true;
                        rescue_cooldown_end = t + rescue_cooldown_sec;
                        rescue_completed = true;
                        dernier_temps_recuperation = temps_sauvetage ;
                        derniere_altitude_recuperation = avion.get_altitude() ;
                    }
                }
                
                // Vérifie l'échec du sauvetage (après 60s ou phase terminée)
                if ( (config.getString("rescue_strategy") != "GEN_GIVE" || config.getString("rescue_strategy") != "GEN_FIND" ) 
                    && (temps_sauvetage >= temps_max_essai_sauvetage && !rescue_successful)) {  //(config.getString("rescue_strategy") != "GEN_GIVE") &&  
                    if (sim_logs_enabled()) {
                        std::cout << "[T=" << t << "s] ✗ SAUVETAGE ECHOUE (abandon après 60s)" << std::endl;
                        std::cout << "  - Altitude: " << avion.get_altitude() << " m" << std::endl;
                        std::cout << "  - Descente: " << avion.get_vitesse_z() << " m/s" << std::endl;
                        std::cout << "  - Assiette: " << (avion.get_pitch() * RAD_TO_DEG) << " deg" << std::endl;
                    }
                    rescue_activated = false;  // Abandon du sauvetage
                    rescue_vz_positive_time = 0.0;
                }
            }
        }
        
       
        if (!rescue_hold_active && !rescue_cooldown_active && !rescue_activated) {
            
            // Phase 3: Stall alarm - pilot inputs aggressive elevator command (EXPONENTIAL ramp)
            if (t >= test_cmd_start && t < test_cmd_end) {
                double ramp_duration = 45.0;  // seconds to reach ~99% of final value
                double t_relative = t - test_cmd_start;
                
                // Progressive ramp: slow start, faster increase toward the end (0s -> 45s)
                double ramp = std::min(1.0, std::max(0.0, t_relative / ramp_duration));
                double exp_factor = ramp * ramp;
                
                if (!std::isnan(test_cmd_profondeur)) {
                    avion.get_controle().set_commande_profondeur(test_cmd_profondeur * exp_factor);
                }
                if (!std::isnan(test_cmd_thrust)) {
                    avion.get_controle().set_commande_thrust(test_cmd_thrust);
                }
                
            }
        }


        double n_factor = avion.get_portance() / (avion.get_masse() * config.getDouble("g"));
        
        csv << t << ',' 
            << avion.get_x() << ',' << avion.get_y() << ',' << avion.get_altitude() << ','
            << avion.get_vitesse_x() << ',' << avion.get_vitesse_y() << ',' << avion.get_vitesse_z() << ','
            << avion.get_roll() << ',' << avion.get_pitch() << ',' << avion.get_yaw() << ','
            << avion.get_M_pitch() << ',' << avion.get_forces().M_thrust << ','
            << avion.get_Fx() << ',' << avion.get_Fy() << ',' << avion.get_Fz() << ','
            << avion.get_portance() << ',' << avion.get_trainee() << ',' << avion.get_traction() << ','
            << avion.get_aero().C_L << ',' << avion.get_aero().C_D << ',' << avion.get_aero().C_m << ','
            << speed << ',' << AoA_deg << ',' << avion.get_cmd_profondeur() << ','
            << avion.get_controle().get_cmd_thrust() << ',' << alpha << ','
            << avion.get_aero().get_delta_profondeur() << ',' << n_factor << '\n';
        
        if (avion.get_altitude() <= 0) {
            if (sim_logs_enabled()) {
                cout << "Crash !" << endl;
                cout << "CRASH z=" << avion.get_altitude()
                    << " vz=" << avion.get_vitesse_z()
                    << " pitch=" << avion.get_pitch() << endl;
            }
            crash_time = t;
            dernier_temps_recuperation = t - temps_debut_sauvetage ;
            derniere_altitude_recuperation = avion.get_altitude() ;
            break;
        }
    }
    
    csv.close();
    if (sim_logs_enabled()) {
        std::cout << "Simulation enregistree dans : " << fichier_sortie << std::endl;
    }
    return crash_time;
}
