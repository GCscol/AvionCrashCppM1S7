#include "Avion.h"
#include "Constantes.h"
#include "Simulateur.h"
#include "Integration.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <iomanip>

using namespace std;
using namespace Physique;

// Analyzes system energy evolution: kinetic, potential, mechanical energy and work of forces
class AnalyseurEnergie {
private:
    struct EnergyState {
        double time;
        double altitude;
        double vitesse;
        double energie_cinetique;
        double energie_potentielle;
        double energie_mecanique;
        double travail_traction;
        double travail_trainee;
        double bilan_energetique;       // E_mec - (travail_traction - travail_trainee)
        double portance;
        double trainee;
        double traction;
        double alpha_deg;
    };

    vector<EnergyState> data;
    string fichier_sortie;
    double masse;
    double altitude_ref;
    double energie_mecanique_init;
    double travail_traction_cumul = 0.0;
    double travail_trainee_cumul = 0.0;

public:
    AnalyseurEnergie(const string& output_file, double mass) 
        : fichier_sortie(output_file), masse(mass), energie_mecanique_init(0.0) {}

    // Record energy state at time t
    void enregistrer_etat(double t, const Avion& avion) {
        double altitude = avion.get_altitude();
        double vitesse = avion.get_etat().get_vitesse_norme();
        double portance = avion.get_portance();
        double trainee = avion.get_trainee();
        double traction = avion.get_traction();
        double alpha = avion.get_etat().get_alpha();

        double Ec = 0.5 * masse * vitesse * vitesse;
        double Ep = masse * g * altitude;
        double E_mec = Ec + Ep;

        // Work calculation: dW ≈ F·v·dt
        double dt = 0.01;
        travail_traction_cumul += traction * vitesse * dt;
        travail_trainee_cumul -= trainee * vitesse * dt;
        double travail_net_total = travail_traction_cumul + travail_trainee_cumul;
        double bilan = (E_mec - energie_mecanique_init) - travail_net_total;

        EnergyState state = {
            t,
            altitude,
            vitesse,
            Ec,
            Ep,
            E_mec,
            travail_traction_cumul,
            travail_trainee_cumul,
            bilan,
            portance,
            trainee,
            traction,
            alpha * RAD_TO_DEG
        };

        data.push_back(state);

        // Initialize reference energy at first step
        if (t <= 0.02) {
            energie_mecanique_init = E_mec;
        }
    }

    void generer_rapport_csv() {
        ofstream csv(fichier_sortie);
        if (!csv.is_open()) {
            cerr << "Erreur: impossible d'ouvrir " << fichier_sortie << endl;
            return;
        }

        // CSV header
        csv << "time_s,altitude_m,vitesse_ms,Ec_J,Ep_J,E_mec_J,"
            << "travail_traction_J,travail_trainee_J,bilan_energetique_J,"
            << "portance_N,trainee_N,traction_N,alpha_deg\n";

        // CSV data rows
        for (const auto& state : data) {
            csv << fixed << setprecision(6)
                << state.time << ','
                << state.altitude << ','
                << state.vitesse << ','
                << state.energie_cinetique << ','
                << state.energie_potentielle << ','
                << state.energie_mecanique << ','
                << state.travail_traction << ','
                << state.travail_trainee << ','
                << state.bilan_energetique << ','
                << state.portance << ','
                << state.trainee << ','
                << state.traction << ','
                << state.alpha_deg << '\n';
        }

        csv.close();
        cout << "Rapport energétique enregistre: " << fichier_sortie << endl;
        generer_resume();
    }

    // Generate energy analysis statistics summary
    void generer_resume() {
        if (data.empty()) return;

        double Ec_init = data[0].energie_cinetique;
        double Ec_final = data.back().energie_cinetique;
        double Ec_max = Ec_init;
        double Ec_min = Ec_init;

        double Ep_init = data[0].energie_potentielle;
        double Ep_final = data.back().energie_potentielle;

        double E_mec_init = data[0].energie_mecanique;
        double E_mec_final = data.back().energie_mecanique;
        double perte_energie = E_mec_init - E_mec_final;
        double perte_percent = (perte_energie / abs(E_mec_init)) * 100;

        double travail_traction_total = data.back().travail_traction;
        double travail_trainee_total = data.back().travail_trainee;

        for (const auto& state : data) {
            Ec_max = max(Ec_max, state.energie_cinetique);
            Ec_min = min(Ec_min, state.energie_cinetique);
        }

        cout << "\n========== RESUME ANALYSE ENERGETIQUE ==========" << endl;
        cout << fixed << setprecision(2);
        cout << "Duree simulation: " << data.back().time << " s" << endl;

        cout << "\n--- ENERGIE MECANIQUE TOTALE ---" << endl;
        cout << "  Initiale: " << E_mec_init / 1e9 << " GJ" << endl;
        cout << "  Finale: " << E_mec_final / 1e9 << " GJ" << endl;
        cout << "  Perte: " << perte_energie / 1e9 << " GJ (" << perte_percent << "%)" << endl;

        cout << "\n--- TRAVAIL DES FORCES ---" << endl;
        cout << "  Travail traction (cumule): " << travail_traction_total / 1e9 << " GJ" << endl;
        cout << "  Travail trainee (cumule): " << travail_trainee_total / 1e9 << " GJ" << endl;
        cout << "  Travail net: " << (travail_traction_total + travail_trainee_total) / 1e9 << " GJ" << endl;

        cout << "\n--- BILAN ENERGETIQUE ---" << endl;
        cout << "  Variation E_mec: " << (E_mec_final - E_mec_init) / 1e9 << " GJ" << endl;
        cout << "  Travail net attendu: " << (travail_traction_total + travail_trainee_total) / 1e9 << " GJ" << endl;
        cout << "  Difference (perte aero): " << (E_mec_final - E_mec_init - travail_traction_total - travail_trainee_total) / 1e9 << " GJ" << endl;
    }
};

/**
 * Fonction wrapper pour analyser une simulation complète
 */
int analyser_energie_simulation(const string& fichier_csv_sortie,
                                double pas_temps,
                                double duree_sim,
                                double commande_profondeur,
                                double commande_thrust,
                                double cmd_start,
                                double cmd_end,
                                bool use_hysteresis) {
    
    cout << "Demarrage analyse energetique..." << endl;
    cout << "  Modele aerodynamique: " << (use_hysteresis ? "Hysteresis" : "Lineaire") << endl;
    cout << "  Duree: " << duree_sim << "s, dt: " << pas_temps << "s" << endl;
    cout << "  Commande profondeur: " << commande_profondeur << endl;
    cout << "  Commande thrust: " << commande_thrust << endl;

    // Créer et initialiser l'avion
    Avion avion(361.6, 6.6, 200000.0, use_hysteresis);
    avion.initialiser();

    // Initialisation du trim complet (alpha, delta_profondeur ET cmd_thrust)
    double speed = avion.get_vitesse_x();
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
        cmd_thrust_trim = std::max(0.0, std::min(1.0, cmd_thrust_trim));
        avion.get_controle().set_commande_thrust(cmd_thrust_trim);
    }

    // Créer l'analyseur d'énergie
    AnalyseurEnergie analyseur(fichier_csv_sortie, avion.get_masse());

    // Exécuter la simulation pas à pas
    const int steps = static_cast<int>(duree_sim / pas_temps);
    double temps_crash = 0.0;

    for (int i = 0; i < steps; ++i) {
        double t = (i + 1) * pas_temps;
        
        // Apply commands BEFORE updating state
        if (t >= cmd_start && t < cmd_end) {
            if (!std::isnan(commande_profondeur))
                avion.get_controle().set_commande_profondeur(commande_profondeur);
            
            // Fenêtres temporelles spécifiques pour le thrust
            if (!std::isnan(commande_thrust)) {
                if (t >= 100.0 && t < 120.0) {
                    avion.get_controle().set_commande_thrust(0.8);
                } else if (t >= 145.0 && t < 170.0) {
                    avion.get_controle().set_commande_thrust(0.6);
                } else {
                    avion.get_controle().set_commande_thrust(commande_thrust);
                }
            }
        }
        
        // Update state AFTER commands
        avion.mettre_a_jour_etat(pas_temps);
        
        // Enregistrer l'état énergétique
        analyseur.enregistrer_etat(t, avion);
        
        if (avion.get_altitude() <= 0) {
            cout << "Crash detecte à t = " << t << "s" << endl;
            temps_crash = t;
            break;
        }
    }

    // Générer les rapports
    analyseur.generer_rapport_csv();

    return 0;
}

/**
 * Fonction d'analyse d'énergie avec intégration RK4
 */
int analyser_energie_simulation_rk4(const string& fichier_csv_sortie,
                                    double pas_temps,
                                    double duree_sim,
                                    double commande_profondeur,
                                    double commande_thrust,
                                    double cmd_start,
                                    double cmd_end,
                                    bool use_hysteresis) {
    
    cout << "\nDemarrage analyse energetique (RK4)..." << endl;
    cout << "  Methode integration: Runge-Kutta 4eme ordre" << endl;
    cout << "  Modele aerodynamique: " << (use_hysteresis ? "Hysteresis" : "Lineaire") << endl;
    cout << "  Duree: " << duree_sim << "s, dt: " << pas_temps << "s" << endl;
    cout << "  Commande profondeur: " << commande_profondeur << endl;
    cout << "  Commande thrust: " << commande_thrust << endl;

    // Créer et initialiser l'avion
    Avion avion(361.6, 6.6, 200000.0, use_hysteresis);
    avion.initialiser();

    // Initialisation du trim complet
    double speed = avion.get_vitesse_x();
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
        cmd_thrust_trim = std::max(0.0, std::min(1.0, cmd_thrust_trim));
        avion.get_controle().set_commande_thrust(cmd_thrust_trim);
    }

    // Créer l'analyseur d'énergie
    AnalyseurEnergie analyseur(fichier_csv_sortie, avion.get_masse());

    // Exécuter la simulation pas à pas avec RK4
    const int steps = static_cast<int>(duree_sim / pas_temps);

    for (int i = 0; i < steps; ++i) {
        double t = (i + 1) * pas_temps;
        
        // Appliquer les commandes AVANT l'intégration RK4
        if (t >= cmd_start && t < cmd_end) {
            if (!std::isnan(commande_profondeur))
                avion.get_controle().set_commande_profondeur(commande_profondeur);
            
            // Fenêtres temporelles spécifiques pour le thrust
            if (!std::isnan(commande_thrust)) {
                if (t >= 100.0 && t < 120.0) {
                    avion.get_controle().set_commande_thrust(0.8);
                } else if (t >= 145.0 && t < 170.0) {
                    avion.get_controle().set_commande_thrust(0.6);
                } else {
                    avion.get_controle().set_commande_thrust(commande_thrust);
                }
            }
        }
        
        // RK4 integration (much more accurate than Euler)
        Integration::RK4(avion, pas_temps);
        
        // Enregistrer l'état énergétique
        analyseur.enregistrer_etat(t, avion);
        
        if (avion.get_altitude() <= 0) {
            cout << "Crash detecte à t = " << t << "s" << endl;
            break;
        }
    }

    // Générer les rapports
    analyseur.generer_rapport_csv();

    return 0;
}

// Simple function to execute complete energy analysis
// Can be called directly from main()
int main_energie_analysis() {
    // Analyze simulation with linear model
    cout << "ANALYSE ENERGIE - MODELE LINEAIRE" << endl;
    analyser_energie_simulation("output/energie_simulation_linear.csv",
                               0.01,   // dt
                               600.0,  // duration
                               -0.7,   // cmd_profondeur
                               0.6,    // cmd_thrust
                               100.0,  // cmd_start
                               500.0,  // cmd_end
                               false); // linear model

    cout << "ANALYSE ENERGIE - MODELE HYSTERESIS" << endl;
    analyser_energie_simulation("output/energie_simulation_hysteresis.csv",
                               0.01,   // dt
                               600.0,  // duration
                               -0.4,   // cmd_profondeur
                               1.0,    // cmd_thrust
                               50.0,   // cmd_start
                               600.0,  // cmd_end
                               true);  // hysteresis model
    
    return 0;
}

/**
 * Exécute l'analyse d'énergie avec intégration RK4
 * pour comparaison avec la méthode par défaut
 */
int main_energie_analysis_rk4() {
    cout << "\n╔════════════════════════════════════════════════╗" << endl;
    cout << "║  ANALYSE ÉNERGIE RK4 - MODÈLE LINÉAIRE       ║" << endl;
    cout << "╚════════════════════════════════════════════════╝" << endl;
    analyser_energie_simulation_rk4("output/energie_simulation_linear_rk4.csv",
                                   0.01,   // dt
                                   600.0,  // duration
                                   -0.7,   // cmd_profondeur
                                   0.6,    // cmd_thrust
                                   100.0,  // cmd_start
                                   500.0,  // cmd_end
                                   false); // linear model

    cout << "\n╔════════════════════════════════════════════════╗" << endl;
    cout << "║  ANALYSE ÉNERGIE RK4 - MODÈLE HYSTÉRÉSIS     ║" << endl;
    cout << "╚════════════════════════════════════════════════╝" << endl;
    analyser_energie_simulation_rk4("output/energie_simulation_hysteresis_rk4.csv",
                                   0.01,   // dt
                                   600.0,  // duration
                                   -0.4,   // cmd_profondeur
                                   1.0,    // cmd_thrust
                                   50.0,   // cmd_start
                                   600.0,  // cmd_end
                                   true);  // hysteresis model
    
    return 0;
}
