#include "Avion.h"
#include "AnalyseurEnveloppeVol.h"
#include "Simulateur.h"
#include "Constantes.h"
#include "AnalyseurEnergie.h"
#include <iostream>
#include <iomanip>

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


    // cout << "ANALYSE ENERGIE (Integration Euler)" << endl;
    // main_energie_analysis();
    // cout << "ANALYSE ENERGIE (Integration RK4)" << endl;
    // main_energie_analysis_rk4();



    // Scenario with rescue enabled
    std::cout << "\nSCENARIO 1: PROGRESSIVE RECOVERY\n" << std::endl;
    
    Avion avion_scenario1(361.6, 6.6, 140000.0, true);
    avion_scenario1.initialiser();
    Simulateur sim_scenario1(avion_scenario1, 0.01, 600.0, "output/scenario1_progressif.csv",
                          -0.4, 1.0, 50.0, 600.0, true);
    double crash_time_s1 = sim_scenario1.executer();
    std::cout << "Scenario 1 crash time: " << crash_time_s1 << " s\n" << std::endl;

    // Baseline without rescue
    std::cout << "\nBASELINE: NO RESCUE SYSTEM\n" << std::endl;
    
    Avion avion_no_rescue(361.6, 6.6, 140000.0, true);
    avion_no_rescue.initialiser();
    Simulateur sim_no_rescue(avion_no_rescue, 0.01, 600.0, "output/baseline_no_rescue.csv",
                          -0.4, 1.0, 50.0, 600.0, false);
    double crash_time_baseline = sim_no_rescue.executer();
    std::cout << "Baseline crash time: " << crash_time_baseline << " s\n" << std::endl;

    // Compare results
    std::cout << "\nCOMPARATIVE SUMMARY\n" << std::endl;
    std::cout << "Scenario 1 (Progressive):  " << crash_time_s1 << " s" << std::endl;
    std::cout << "Baseline (No rescue):      " << crash_time_baseline << " s" << std::endl;
    
    if (!std::isnan(crash_time_s1) && !std::isnan(crash_time_baseline)) {
        double gain_s1 = crash_time_s1 - crash_time_baseline;
        std::cout << "\nGain Scenario 1: " << gain_s1 << " s (" 
                  << (gain_s1 > 0 ? "+" : "") << std::fixed << std::setprecision(1) 
                  << (gain_s1 / crash_time_baseline * 100) << "%)" << std::endl;
    }
    

    return 0;
}