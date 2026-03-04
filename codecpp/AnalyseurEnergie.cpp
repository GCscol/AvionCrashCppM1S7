#include "AnalyseurEnergie.h"
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
using namespace Math;

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
        double Ep = masse * config.getDouble("g") * altitude;
        double E_mec = Ec + Ep;

        // Work calculation: dW ≈ F·v·dt
        double dt = 0.01;
        travail_traction_cumul += traction * vitesse * cos(alpha) * dt; // Il manquerait un cos(alpha)
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
            alpha * Math::RAD_TO_DEG
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
        
        if (!data.empty()) {
            double E_mec_init = data[0].energie_mecanique;
            double E_mec_final = data.back().energie_mecanique;
            double travail_traction_total = data.back().travail_traction;
            double travail_trainee_total = data.back().travail_trainee;
            double travail_net = travail_traction_total + travail_trainee_total;
            double delta_E_mec = E_mec_final - E_mec_init;
            double bilan = delta_E_mec - travail_net;
            
            cout << "--- Energy Balance ---" << endl;
            cout << "Check: Delta_E_mec - Work_net = " << bilan / 1e9 << " GJ" << endl;
        }
    }


};

/**
 * Fonction wrapper pour analyser une simulation complète
 */
void analyser_energie_simulation(const ParamsSimulationEnergie& params) {

    // Créer et initialiser l'avion
    Avion avion(361.6, 6.6, 200000.0, params.use_hysteresis);
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
    AnalyseurEnergie analyseur(params.fichier_sortie, avion.get_masse());

    // Exécuter la simulation pas à pas
    const int steps = static_cast<int>(params.duree / params.pas_temps);

    for (int i = 0; i < steps; ++i) {
        double t = (i + 1) * params.pas_temps;
        
        // Apply commands BEFORE updating state
        if (t >= params.cmd_debut && t < params.cmd_fin) {
            if (!std::isnan(params.cmd_profondeur))
                avion.get_controle().set_commande_profondeur(params.cmd_profondeur);
            
            // Fenêtres temporelles spécifiques pour le thrust
            if (!std::isnan(params.cmd_thrust)) {
                if (t >= 100.0 && t < 120.0) {
                    avion.get_controle().set_commande_thrust(0.8);
                } else if (t >= 145.0 && t < 170.0) {
                    avion.get_controle().set_commande_thrust(0.6);
                } else {
                    avion.get_controle().set_commande_thrust(params.cmd_thrust);
                }
            }
        }
        
        // Update state AFTER commands
        avion.mettre_a_jour_etat(params.pas_temps);
        
        // Enregistrer l'état énergétique
        analyseur.enregistrer_etat(t, avion);
        
        if (avion.get_altitude() <= 0) {
            break;
        }
    }

    analyseur.generer_rapport_csv();
}

/**
 * Fonction d'analyse d'énergie avec intégration RK4
 */
void analyser_energie_simulation_rk4(const ParamsSimulationEnergie& params) {

    // Créer et initialiser l'avion
    Avion avion(361.6, 6.6, 200000.0, params.use_hysteresis);
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
    AnalyseurEnergie analyseur(params.fichier_sortie, avion.get_masse());

    // Exécuter la simulation pas à pas avec RK4
    const int steps = static_cast<int>(params.duree / params.pas_temps);

    for (int i = 0; i < steps; ++i) {
        double t = (i + 1) * params.pas_temps;
        
        // Appliquer les commandes AVANT l'intégration RK4
        if (t >= params.cmd_debut && t < params.cmd_fin) {
            if (!std::isnan(params.cmd_profondeur))
                avion.get_controle().set_commande_profondeur(params.cmd_profondeur);
            
            // Fenêtres temporelles spécifiques pour le thrust
            if (!std::isnan(params.cmd_thrust)) {
                if (t >= 100.0 && t < 120.0) {
                    avion.get_controle().set_commande_thrust(0.8);
                } else if (t >= 145.0 && t < 170.0) {
                    avion.get_controle().set_commande_thrust(0.6);
                } else {
                    avion.get_controle().set_commande_thrust(params.cmd_thrust);
                }
            }
        }
        
        // RK4 integration
        Integration::RK4(avion, params.pas_temps);
        
        // Enregistrer l'état énergétique
        analyseur.enregistrer_etat(t, avion);
        
        if (avion.get_altitude() <= 0) {
            break;
        }
    }

    analyseur.generer_rapport_csv();
}

void main_energie_analysis() {
    ParamsSimulationEnergie params_lin{
        "output/energie_simulation_linear.csv",
        0.01,      // pas_temps
        600.0,     // duree
        -0.7,      // cmd_profondeur
        0.6,       // cmd_thrust
        100.0,     // cmd_debut
        500.0,     // cmd_fin
        false      // use_hysteresis
    };
    analyser_energie_simulation(params_lin);
}

/**
 * Exécute l'analyse d'énergie avec intégration RK4
 * pour comparaison avec la méthode par défaut
 */
void main_energie_analysis_rk4() {
    ParamsSimulationEnergie params_lin_rk4{
        "output/energie_simulation_linear_rk4.csv",
        0.01,      // pas_temps
        600.0,     // duree
        -0.7,      // cmd_profondeur
        0.6,       // cmd_thrust
        100.0,     // cmd_debut
        500.0,     // cmd_fin
        false      // use_hysteresis
    };
    analyser_energie_simulation_rk4(params_lin_rk4);
}