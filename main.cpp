#include "Avion.h"
#include "AnalyseurEnveloppeVol.h"
#include "Simulateur.h"
#include "Constantes.h"
#include "AnalyseurEnergie.h"
#include <iostream>

using namespace std;

int run_batch(double p_min, double p_max, double p_step,
              double t_min, double t_max, double t_step,
              bool useHysteresis,
              double sim_dt = 0.01, double sim_duration = 600.0,
              double cmd_start = 100.0, double cmd_end = 500.0);

int main() {
	// #ifdef MODE_SIMULATION
    // Avion avion(361.6, 6.6, 140000.0, false); // linear aerodynamic model
    // avion.initialiser();
    // Simulateur sim(avion, 0.01, 600.0, "simulation_full.csv", -0.7, 0.6, 100, 600);
    // sim.executer();

    // Avion avion(361.6, 6.6, 140000.0, true); // hysteresis aerodynamic model
    // avion.initialiser();
    // Simulateur sim(avion, 0.01, 600.0, "simulation_hyst.csv", -0.4, 1.0, 50, 600);
    // sim.executer();


    // AnalyseurEnveloppeVol analyseur(avion);
    // analyseur.analyser_limites_vitesse();


    // run_batch(-1.0, -0.0, 0.1, 0.0, 1.0, 0.1, true, 0.01, 600.0, 100.0, 500.0);


    // Avion avion_rescue(361.6, 6.6, 140178.9, true);
    // avion_rescue.initialiser();
    // Simulateur sim_rescue(avion_rescue, 0.01, 600.0, "output/test_sauvetage.csv",
    //                       -0.6,     // cmd_profondeur 
    //                       0.7,      // cmd_thrust
    //                       50.0,     // cmd_start 
    //                       300.0,    // cmd_end
    //                       true);    // ENABLE_RESCUE = true
    // sim_rescue.executer();

    // Avion avion_no_rescue(361.6, 6.6, 140178.9, true);
    // avion_no_rescue.initialiser();
    // Simulateur sim_no_rescue(avion_no_rescue, 0.01, 600.0, "output/test_no_sauvetage.csv",
    //                       -0.6,     // cmd_profondeur 
    //                       0.7,      // cmd_thrust
    //                       50.0,     // cmd_start 
    //                       600.0,    // cmd_end
    //                       false);    // ENABLE_RESCUE = false
    // sim_no_rescue.executer();


    // Analyse d'énergie avec intégration par défaut
    cout << "\n╔════════════════════════════════════════════════╗" << endl;
    cout << "║   PHASE 1: ANALYSE ÉNERGIE (Intégration défaut) ║" << endl;
    cout << "╚════════════════════════════════════════════════╝" << endl;
    main_energie_analysis();
    
    // Analyse d'énergie avec RK4 pour comparaison
    cout << "\n╔════════════════════════════════════════════════╗" << endl;
    cout << "║   PHASE 2: ANALYSE ÉNERGIE (Intégration RK4)   ║" << endl;
    cout << "╚════════════════════════════════════════════════╝" << endl;
    main_energie_analysis_rk4();

    return 0;
}