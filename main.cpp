#include "Avion.h"
#include "AnalyseurEnveloppeVol.h"
#include "Simulateur.h"
#include "Constantes.h"
#include "AnalyseurEnergie.h"
#include "SauvetageAvion.h"
#include "test_initialization_range.h"
#include <iostream>
#include <iomanip>

using namespace std;

extern int CURRENT_STRATEGY;

int run_batch(double p_min, double p_max, double p_step,
              double t_min, double t_max, double t_step,
              bool useHysteresis,
              double sim_dt = 0.01, double sim_duration = 600.0,
              double cmd_start = 100.0, double cmd_end = 500.0);

int main() {

    //if (config.hasOperations("RUN_BATCH")) batch
    //if (config.hasOperations("ENVELOPPE"))  enveloppe
    //if (config.hasOperations("ENERGIE"))    energie
    //if (config.hasOperations("SIMULATION")) simu


    // Load configuration into global config variable
    config.chargerDepuisFichier("Config.txt");
    config.completer();

    config.exporter("Config_simulation_full.txt");


    // TEST DE L'INITIALISATION AVEC DIFFÉRENTES VITESSES ET ALTITUDES
    // test_initialization_range(100.0, 500.0, 5.0,   // vitesse: 230-250 m/s par pas de 5
    //                          10000.0, 20000.0, 500.0,  // altitude: 10000-13000 m par pas de 250
    //                          false, 600.0, "output/init_range.csv");
    
	// #ifdef MODE_SIMULATION
    Avion avion(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis")); // linear aerodynamic model // 140000
    // avion.initialiser(240.0, 10670.0);  // croisière
    // // avion.initialiser(220.0, 11280)  // 11280
    // Simulateur sim(avion, config.getDouble("dt"), config.getDouble("duree"), "simulation_full.csv", config.getDouble("cmd_profondeur"), config.getDouble("cmd_thrust"), config.getDouble("cmd_start"), config.getDouble("cmd_end")); // -0.32
    // sim.executer();


    // Avion avion(361.6, 6.6, 140000.0, true); // hysteresis aerodynamic model
    // avion.initialiser();
    // Simulateur sim(avion, 0.01, 600.0, "simulation_hyst.csv", -0.4, 1.0, 50, 600);
    // sim.executer();


    // AnalyseurEnveloppeVol analyseur(avion);
    // analyseur.analyser_limites_vitesse();


    // run_batch(-1.0, -0.0, 0.1, 0.0, 1.0, 0.1, true, 0.01, 600.0, 100.0, 500.0);


    cout << "ANALYSE ENERGIE (Integration Euler)" << endl;
    main_energie_analysis();
    cout << "ANALYSE ENERGIE (Integration RK4)" << endl;
    main_energie_analysis_rk4();



    // Baseline without rescue
    // std::cout << "\nBASELINE: NO RESCUE SYSTEM\n" << std::endl;
    
    // Avion avion_no_rescue(361.6, 6.6, 140000.0, true);
    // avion_no_rescue.initialiser();
    // Simulateur sim_no_rescue(avion_no_rescue, 0.01, 600.0, "output/baseline_no_rescue.csv",
    //                       -0.4, 1.0, 50.0, 600.0, false);
    // double crash_time_baseline = sim_no_rescue.executer();
    // std::cout << "Baseline crash time: " << crash_time_baseline << " s\n" << std::endl;

    // // Test STRATEGY 0: Thrust reduction first
    // std::cout << "\nSTRATEGY 0: THRUST REDUCTION FIRST (then pitch)\n" << std::endl;
    // CURRENT_STRATEGY = 0;
    
    // Avion avion_s0(361.6, 6.6, 140000.0, true);
    // avion_s0.initialiser();
    // Simulateur sim_s0(avion_s0, 0.01, 600.0, "output/strategy0_thrust_first.csv",
    //                   -0.4, 1.0, 50.0, 600.0, true);
    // double crash_time_s0 = sim_s0.executer();
    // std::cout << "Strategy 0 crash time: " << crash_time_s0 << " s\n" << std::endl;

    // // Test STRATEGY 1: Profile reduction first
    // std::cout << "\nSTRATEGY 1: PROFILE REDUCTION FIRST (then thrust)\n" << std::endl;
    // CURRENT_STRATEGY = 1;
    
    // Avion avion_s1(361.6, 6.6, 140000.0, true);
    // avion_s1.initialiser();
    // Simulateur sim_s1(avion_s1, 0.01, 600.0, "output/strategy1_profile_first.csv",
    //                   -0.4, 1.0, 50.0, 600.0, true);
    // double crash_time_s1 = sim_s1.executer();
    // std::cout << "Strategy 1 crash time: " << crash_time_s1 << " s\n" << std::endl;

    // // Test STRATEGY 2: Both simultaneously
    // std::cout << "\nSTRATEGY 2: SIMULTANEOUS REDUCTION (thrust + pitch)\n" << std::endl;
    // CURRENT_STRATEGY = 2;
    
    // Avion avion_s2(361.6, 6.6, 140000.0, true);
    // avion_s2.initialiser();
    // Simulateur sim_s2(avion_s2, 0.01, 600.0, "output/strategy2_simultaneous.csv",
    //                   -0.4, 1.0, 50.0, 600.0, true);
    // double crash_time_s2 = sim_s2.executer();
    // std::cout << "Strategy 2 crash time: " << crash_time_s2 << " s\n" << std::endl;

    // // Comparative summary
    // std::cout << "\n============ COMPARATIVE SUMMARY ============\n" << std::endl;
    // std::cout << "Baseline (No rescue):              " << crash_time_baseline << " s" << std::endl;
    // std::cout << "Strategy 0 (Thrust-first):        " << crash_time_s0 << " s";
    // if (!std::isnan(crash_time_s0) && !std::isnan(crash_time_baseline)) {
    //     double gain = crash_time_s0 - crash_time_baseline;
    //     std::cout << " → Gain: " << (gain > 0 ? "+" : "") << std::fixed << std::setprecision(1) << gain << " s";
    // }
    // std::cout << std::endl;
    
    // std::cout << "Strategy 1 (Profile-first):       " << crash_time_s1 << " s";
    // if (!std::isnan(crash_time_s1) && !std::isnan(crash_time_baseline)) {
    //     double gain = crash_time_s1 - crash_time_baseline;
    //     std::cout << " → Gain: " << (gain > 0 ? "+" : "") << std::fixed << std::setprecision(1) << gain << " s";
    // }
    // std::cout << std::endl;
    
    // std::cout << "Strategy 2 (Simultaneous):        " << crash_time_s2 << " s";
    // if (!std::isnan(crash_time_s2) && !std::isnan(crash_time_baseline)) {
    //     double gain = crash_time_s2 - crash_time_baseline;
    //     std::cout << " → Gain: " << (gain > 0 ? "+" : "") << std::fixed << std::setprecision(1) << gain << " s";
    // }
    // std::cout << std::endl;

    return 0;
}