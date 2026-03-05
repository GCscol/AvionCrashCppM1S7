#include "Avion.h"
#include "AnalyseurEnveloppeVol.h"
#include "Simulateur.h"
#include "Constantes.h"
#include "AnalyseurEnergie.h"
#include "SauvetageAvion.h"
#include "test_initialization_range.h"
#include "batch_runner.h"
#include <iostream>
#include <iomanip>

using namespace std;


int main() {

    // Load configuration into global config variable
    config.chargerDepuisFichier("Config.txt");
    config.completer();

    config.exporter(check_output_file(config.getString("config_file")));  // Vérifie export avant d'écraser


    // TEST DE L'INITIALISATION AVEC DIFFÉRENTES VITESSES ET ALTITUDES

    if (config.hasOperations("TEST_INITIALISATION")) {   // TOUJOURS VALABLE ???????????????????????????????????????
        test_initialization_range(100.0, 500.0, 5.0,   // vitesse: 230-250 m/s par pas de 5
                                10000.0, 20000.0, 500.0,  // altitude: 10000-13000 m par pas de 250
                                false, 600.0, check_output_file("output/init_range.csv"));
    }
    
    if (config.hasOperations("SIMULATION")) {
        Avion avion(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis")); // linear aerodynamic model // 140000
    // avion.initialiser(240.0, 10670.0);  // croisière
                                        // 11280
        avion.initialiser(config.getDouble("vx_ini"), config.getDouble("z_ini"));  // 11280
        Simulateur sim(avion, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file(config.getString("output_file")), 
                        config.getDouble("cmd_profondeur"), config.getDouble("cmd_thrust"), 
                        config.getDouble("cmd_start"), config.getDouble("cmd_end"),
                        config.getBool("enable_rescue_system")); // -0.32
        sim.executer();
    }   

    // Avion avion(361.6, 6.6, 140000.0, true); // hysteresis aerodynamic model
    // avion.initialiser();
    // Simulateur sim(avion, 0.01, 600.0, "simulation_hyst.csv", -0.4, 1.0, 50, 600);
    // sim.executer();

    if (config.hasOperations("ENVELOPPE")) {
        AnalyseurEnveloppeVol analyseur(avion);
        analyseur.analyser_limites_vitesse();
    }

    if (config.hasOperations("RUN_BATCH")) {
        run_batch(config.getDouble("p_min"), config.getDouble("p_max"), config.getDouble("p_step"), 
                    config.getDouble("t_min"), config.getDouble("t_max"),  config.getDouble("t_step"), 
                    config.getBool("useHysteresis"), 
                    config.getDouble("dt"), config.getDouble("duree"), 
                    config.getDouble("cmd_start"), config.getDouble("cmd_end"));
    }

    if (config.hasOperations("ENERGIE"))  {
        cout << "ANALYSE ENERGIE (Integration Euler)" << endl;
        main_energie_analysis();
        cout << "ANALYSE ENERGIE (Integration RK4)" << endl;
        main_energie_analysis_rk4();
    }

    // Mettre une boucle ?????????????
    if (config.hasOperations("COMPARE_RESCUE_STRATEGIES"))  { // Mettre une boucle ?????????????
        double crash_time_baseline, crash_time_s0, crash_time_s1, crash_time_s2;
        //Baseline without rescue
        {
        std::cout << "\nBASELINE: NO RESCUE SYSTEM\n" << std::endl;
        Avion avion_no_rescue(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis"));
        avion_no_rescue.initialiser();
        Simulateur sim_no_rescue(avion_no_rescue, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file("output/baseline_no_rescue.csv"), 
                        config.getDouble("cmd_profondeur"), config.getDouble("cmd_thrust"), 
                        config.getDouble("cmd_start"), config.getDouble("cmd_end"),
                        false); 
        crash_time_baseline = sim_no_rescue.executer();
        std::cout << "Baseline crash time: " << crash_time_baseline << " s\n" << std::endl;
        }

        // Test STRATEGY 0: Thrust reduction first
        {
        std::cout << "\nSTRATEGY 0: THRUST REDUCTION FIRST (then pitch)\n" << std::endl;
        config.setString("rescue_strategy", "THRUST_FIRST"); 
        
        Avion avion_s0(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis"));
        avion_s0.initialiser();
        Simulateur sim_s0(avion_s0, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file("output/strategy0_thrust_first.csv"), 
                        config.getDouble("cmd_profondeur"), config.getDouble("cmd_thrust"), 
                        config.getDouble("cmd_start"), config.getDouble("cmd_end"),
                        true);
        crash_time_s0 = sim_s0.executer();
        std::cout << "Strategy 0 crash time: " << crash_time_s0 << " s\n" << std::endl;
        }

        // Test STRATEGY 1: Profile reduction first
        {
        std::cout << "\nSTRATEGY 1: PROFILE REDUCTION FIRST (then thrust)\n" << std::endl;
        config.setString("rescue_strategy", "PROFILE_FIRST");
        
        Avion avion_s1(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis"));
        avion_s1.initialiser();
        Simulateur sim_s1(avion_s1, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file("output/strategy1_profile_first.csv"), 
                        config.getDouble("cmd_profondeur"), config.getDouble("cmd_thrust"), 
                        config.getDouble("cmd_start"), config.getDouble("cmd_end"),
                        true);
        crash_time_s1 = sim_s1.executer();
        std::cout << "Strategy 1 crash time: " << crash_time_s1 << " s\n" << std::endl;
        }

        // Test STRATEGY 2: Both simultaneously
        {
        std::cout << "\nSTRATEGY 2: SIMULTANEOUS REDUCTION (thrust + pitch)\n" << std::endl;
        config.setString("rescue_strategy", "SIMULTANEOUS");
        
        Avion avion_s2(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis"));
        avion_s2.initialiser();
        Simulateur sim_s2(avion_s2, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file("output/strategy2_simultaneous.csv"), 
                        config.getDouble("cmd_profondeur"), config.getDouble("cmd_thrust"), 
                        config.getDouble("cmd_start"), config.getDouble("cmd_end"),
                        true);;
        crash_time_s2 = sim_s2.executer();
        std::cout << "Strategy 2 crash time: " << crash_time_s2 << " s\n" << std::endl;
        }

        // Comparative summary
        std::cout << "\n============ COMPARATIVE SUMMARY ============\n" << std::endl;
        std::cout << "Baseline (No rescue):              " << crash_time_baseline << " s" << std::endl;
        std::cout << "Strategy 0 (Thrust-first):        " << crash_time_s0 << " s";
        if (!std::isnan(crash_time_s0) && !std::isnan(crash_time_baseline)) {
            double gain = crash_time_s0 - crash_time_baseline;
            std::cout << " → Gain: " << (gain > 0 ? "+" : "") << std::fixed << std::setprecision(1) << gain << " s";
        }
        std::cout << std::endl;
        
        std::cout << "Strategy 1 (Profile-first):       " << crash_time_s1 << " s";
        if (!std::isnan(crash_time_s1) && !std::isnan(crash_time_baseline)) {
            double gain = crash_time_s1 - crash_time_baseline;
            std::cout << " → Gain: " << (gain > 0 ? "+" : "") << std::fixed << std::setprecision(1) << gain << " s";
        }
        std::cout << std::endl;
        
        std::cout << "Strategy 2 (Simultaneous):        " << crash_time_s2 << " s";
        if (!std::isnan(crash_time_s2) && !std::isnan(crash_time_baseline)) {
            double gain = crash_time_s2 - crash_time_baseline;
            std::cout << " → Gain: " << (gain > 0 ? "+" : "") << std::fixed << std::setprecision(1) << gain << " s";
        }
        std::cout << std::endl;
    }



    return 0;
}