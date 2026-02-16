#include "Simulateur.h"
#include "Avion.h"
#include "Constantes.h"
#include "SauvetageAvion.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

using namespace std;

// a verifier

Simulateur::Simulateur(Avion& av, double pas_temps, double duree,
                                             const string& fichier,
                                             double cmd_profondeur,
                                             double cmd_thrust,
                                             double cmd_start,
                                             double cmd_end,
                                             bool enable_rescue_system)
        : avion(av), dt(pas_temps), temps_total(duree), fichier_sortie(fichier),
            test_cmd_profondeur(cmd_profondeur), test_cmd_thrust(cmd_thrust),
            test_cmd_start(cmd_start), test_cmd_end(cmd_end),
            enable_rescue(enable_rescue_system), temps_debut_sauvetage(-1e6) {}

double Simulateur::executer() {
    using namespace Physique;
    
    // setlocale(LC_ALL, ".utf8");
    
    const int steps = static_cast<int>(temps_total / dt);
    
    std::ofstream csv(fichier_sortie);
    if (!csv.is_open()) {
        std::cerr << "Impossible d'ouvrir " << fichier_sortie << " en écriture\n";
        return std::numeric_limits<double>::quiet_NaN();
    }
    
        // Initialisation du trim complet (alpha, delta_profondeur ET cmd_thrust)
    double speed = avion.get_vitesse_x();
    
    // Utiliser la nouvelle méthode qui converge vers un équilibre complet
    std::pair<double, double> trim_result = avion.calculer_trim_complet(speed);
    double alpha_trim = trim_result.first;
    double delta_trim = trim_result.second;
    
    avion.get_etat().pitch = alpha_trim;
    avion.get_aero().set_delta_profondeur(delta_trim);
    
    double rho = avion.get_env().calculer_rho(avion.get_altitude());
    avion.get_aero().update_from_polar(alpha_trim, delta_trim, 0.0, speed);
    double trainee_trim = avion.get_aero().calculer_trainee(speed, rho);
    double traction_max = avion.calculer_poussee_max(speed, rho, avion.get_altitude());
    
    if (traction_max > 0.0) {
        double cmd_thrust_trim = trainee_trim / traction_max;
        // Limiter à [0, 1]
        cmd_thrust_trim = std::max(0.0, std::min(1.0, cmd_thrust_trim));
        avion.get_controle().set_commande_thrust(cmd_thrust_trim);
    }
    
    csv << "time,"
        << "x,y,z,vx,vy,vz,"
        << "roll,pitch,yaw,"
        << "M_pitch,M_thrust,"
        << "Fx,Fy,Fz,portance,trainee,traction,"
        << "Cl,Cd,Cm,"
        << "speed,AoA_deg,cmd_profondeur,alpha,delta_profondeur,n_factor\n";
    
        // Determine if this is the control_test mode (special command sequence)

        // Boucle de simulation
    double crash_time = std::numeric_limits<double>::quiet_NaN();
    bool rescue_activated = false;
    bool rescue_successful = false;
    int rescue_strategy_used = 0;
    EtatCinematique etat_avant_sauvetage;
    int rescue_activation_count = 0;
    
    for (int i = 0; i < steps; ++i) {
        double t = (i + 1) * dt;
        avion.mettre_a_jour_etat(dt);
        
        double speed = avion.get_etat().get_vitesse_norme();
        double alpha = avion.get_etat().get_alpha();
        double AoA_deg = alpha * RAD_TO_DEG;
        
        // ============ GESTION DU SAUVETAGE AUTOMATIQUE ============
        if (enable_rescue) {
            // Évalue l'état de l'avion
            SauvetageAvion::EtatSauvetage etat_sauvetage = SauvetageAvion::evaluer_etat(
                avion.get_etat(), t, temps_debut_sauvetage);
            
            // Si en descente critique ET pas encore en sauvetage
            if (etat_sauvetage.en_descente_critique && !rescue_activated) {
                rescue_activated = true;
                rescue_activation_count++;
                temps_debut_sauvetage = t;
                etat_avant_sauvetage = avion.get_etat();
                
                std::cout << "\n========== ACTIVATION SAUVETAGE AUTOMATIQUE (#" << rescue_activation_count << ") ==========" << std::endl;
                std::cout << "[T=" << t << "s] Situation critique detectee:" << std::endl;
                std::cout << "  - Altitude: " << avion.get_altitude() << " m" << std::endl;
                std::cout << "  - Taux descente: " << avion.get_vitesse_z() << " m/s" << std::endl;
                std::cout << "  - Assiette: " << (avion.get_pitch() * RAD_TO_DEG) << " deg" << std::endl;
            }
            
            // Applique les commandes de sauvetage si actif
            if (rescue_activated) {
                double temps_sauvetage = t - temps_debut_sauvetage;
                
                // Auto-sélection de la stratégie selon l'altitude
                if (rescue_strategy_used == 0) {
                    if (avion.get_altitude() < 300.0) {
                        rescue_strategy_used = 2;  // Poussée max
                    } else if (avion.get_altitude() < 800.0) {
                        rescue_strategy_used = 3;  // Manoeuvre coordonnée
                    } else {
                        rescue_strategy_used = 1;  // Pull-up progressif
                    }
                    
                    std::cout << "[T=" << t << "s] Strategie choisie: ";
                    switch(rescue_strategy_used) {
                        case 1:
                            std::cout << "PULL-UP PROGRESSIF (doux)" << std::endl;
                            break;
                        case 2:
                            std::cout << "POUSSÉE MAX (agressif)" << std::endl;
                            break;
                        case 3:
                            std::cout << "MANOEUVRE COORDONNÉE (équilibrée)" << std::endl;
                            break;
                    }
                }
                
                // Applique les commandes 
                SauvetageAvion::EtatSauvetage etat_sauvetage = SauvetageAvion::evaluer_etat(
                    avion.get_etat(), t, temps_debut_sauvetage);
                std::pair<double, double> cmds = SauvetageAvion::appliquer_sauvetage(etat_sauvetage, rescue_strategy_used);
                
                avion.get_controle().set_commande_profondeur(cmds.first);
                avion.get_controle().set_commande_thrust(cmds.second);

                // Vérifie si le sauvetage a réussi
                if (temps_sauvetage >= 2.0) {
                    rescue_successful = SauvetageAvion::verifier_succes_sauvetage(
                        avion.get_etat(), etat_avant_sauvetage, temps_sauvetage);
                    
                    if (rescue_successful) {
                        std::cout << "[T=" << t << "s] ✓ SAUVETAGE REUSSI" << std::endl;
                        std::cout << "  - Altitude retrouvée: " << avion.get_altitude() << " m" << std::endl;
                        std::cout << "  - Descente ralentie: " << avion.get_vitesse_z() << " m/s" << std::endl;
                        rescue_activated = false;  // Fin du sauvetage
                    }
                }
                
                // Vérifie l'échec du sauvetage (descente continue)
                if (temps_sauvetage >= 30.0 && !rescue_successful) {
                    std::cout << "[T=" << t << "s] ✗ SAUVETAGE ECHOUE (abandon après 30s)" << std::endl;
                    std::cout << "  - Altitude: " << avion.get_altitude() << " m" << std::endl;
                    std::cout << "  - Descente: " << avion.get_vitesse_z() << " m/s" << std::endl;
                    std::cout << "  - Assiette: " << (avion.get_pitch() * RAD_TO_DEG) << " deg" << std::endl;
                    rescue_activated = false;  // Abandon du sauvetage
                }
            }
        }
        
        // ========== FIN GESTION SAUVETAGE ==========
        
        // Commandes pilote: either use provided test commands or fallback
        if (!rescue_activated && (!std::isnan(test_cmd_profondeur) || !std::isnan(test_cmd_thrust))) {
            if (t >= test_cmd_start && t < test_cmd_end) {
                if (!std::isnan(test_cmd_profondeur))
                    avion.get_controle().set_commande_profondeur(test_cmd_profondeur);
                
                // Commande thrust avec plages de temps spécifiques
                if (!std::isnan(test_cmd_thrust)) {
                    if (t >= 100.0 && t < 120.0) {
                        // Entre 100 et 120s: thrust = 0.8
                        avion.get_controle().set_commande_thrust(0.8);
                    } else if (t >= 145.0 && t < 170.0) {
                        // Entre 145 et 170s: thrust = 0.6
                        avion.get_controle().set_commande_thrust(0.6);
                    } else {
                        // Reste du vol: utiliser la commande définie dans l'argument
                        avion.get_controle().set_commande_thrust(test_cmd_thrust);
                    }
                }
            }
        } else if (!rescue_activated) {
            // original hardcoded test
            if (t >= 100 && t < 500) {
                avion.get_controle().set_commande_profondeur(-0.55);
                avion.get_controle().set_commande_thrust(0.9);
            }
        }



        
        double n_factor = avion.get_portance() / (avion.get_masse() * g);
        
        csv << t << ',' 
            << avion.get_x() << ',' << avion.get_y() << ',' << avion.get_altitude() << ','
            << avion.get_vitesse_x() << ',' << avion.get_vitesse_y() << ',' << avion.get_vitesse_z() << ','
            << avion.get_roll() << ',' << avion.get_pitch() << ',' << avion.get_yaw() << ','
            << avion.get_M_pitch() << ',' << avion.get_forces().M_thrust << ','
            << avion.get_Fx() << ',' << avion.get_Fy() << ',' << avion.get_Fz() << ','
            << avion.get_portance() << ',' << avion.get_trainee() << ',' << avion.get_traction() << ','
            << avion.get_aero().C_L << ',' << avion.get_aero().C_D << ',' << avion.get_aero().C_m << ','
            << speed << ',' << AoA_deg << ',' << avion.get_cmd_profondeur() << ','
            << alpha << ',' << avion.get_aero().get_delta_profondeur() << ',' << n_factor << '\n';
        
        if (avion.get_altitude() <= 0) {
            cout << "Crash !" << endl;
            crash_time = t;
            break;
        }
    }
    
    csv.close();
    std::cout << "Simulation enregistree dans : " << fichier_sortie << std::endl;
    return crash_time;
}
