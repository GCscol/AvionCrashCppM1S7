#include "Avion.h"
#include "AnalyseurEnveloppeVol.h"
#include "Simulateur.h"
#include "Constantes.h"
#include "AnalyseurEnergie.h"
#include "SauvetageAvion.h"
#include "OptimiseurSauvetage.h"
#include "test_initialization_range.h"
#include "batch_runner.h"
#include "OptiSauvetageGeneral.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

struct RescueRunResult {
    double activation_altitude;
    std::string strategy;
    bool success;
    double recovery_altitude;
    double crash_time;
};


int main() {

    // Load configuration into global config variable
    config.chargerDepuisFichier("Config.txt");
    config.completer();

    config.exporter(check_output_file(config.getString("config_file")));  // Vérifie export avant d'écraser


    // TEST DE L'INITIALISATION AVEC DIFFÉRENTES VITESSES ET ALTITUDES

    if (config.hasOperations("TEST_INITIALISATION")) {   // TOUJOURS VALABLE ???????????????????????????????????????
        test_initialization_range(230.0, 250.0, 5.0,   // vitesse: 230-250 m/s par pas de 5
                                10000.0, 13000.0, 250.0,  // altitude: 10000-13000 m par pas de 250
                                false, 600.0, check_output_file("output/init_range.csv"));
    }



    if (config.hasOperations("GENERAL_GEN_FIND")) { 
        std::string Initial_quiet_optimizer_logs = config.getString("quiet_optimizer_logs");
        config.setString("quiet_optimizer_logs","true");
        std::srand(static_cast<unsigned>(std::time(nullptr))); // seed differentes pour chaque essai
        // Initialisation de la population vide et de chaque chrosomes se fait dans le constructuer
        OptiSauvetageGeneral gen_opti_strat;
        
        int nbr_generation=gen_opti_strat.get_Nbr_generation();
        int nbr_chrom=gen_opti_strat.get_Nbr_chr();

        std::string Initial_Rescue_Strategy = config.getString("rescue_strategy");
        config.setString("rescue_strategy", "GEN_FIND");


        Avion avion(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis"));
        Simulateur sim(avion, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file(config.getString("output_file")), 
                        config.getDouble("cmd_profondeur"), config.getDouble("cmd_thrust"), 
                        config.getDouble("cmd_start"), config.getDouble("cmd_end"),
                        true); // -0.32
        
        // à refaire
        // Déclaré avant la boucle de générations
        std::string log_path = "output/gen_stats.txt";
        // Efface le fichier au début (sinon append sur un ancien run)
        { std::ofstream f(log_path); f << "generation,chromosome,altitude,temps,fitness\n"; }
        //

        for (int i=0; i<nbr_generation; i++){
            std::cout<<"Generation n°"<<i<<std::endl;

            // A refaire
            std::vector<double> altitudes_gen;
            std::vector<double> temps_gen;
            altitudes_gen.reserve(nbr_chrom);
            temps_gen.reserve(nbr_chrom);
            std::vector<double> fitness_gen;
            fitness_gen.reserve(nbr_chrom);
            //
            for (int k=0; k<nbr_chrom; k++) {
                avion.initialiser(config.getDouble("vx_ini"), config.getDouble("z_ini"));
                sim.executer(&gen_opti_strat.population[k]);

                // a refaire
                double alt  = sim.get_derniere_altitude_recuperation();
                double tps  = sim.get_dernier_temps_recuperation();
                altitudes_gen.push_back(alt);
                temps_gen.push_back(tps);
                //
                gen_opti_strat.population[k].fitness = gen_opti_strat.Eval_Fitness(alt, tps);
                
                //
                fitness_gen.push_back(gen_opti_strat.population[k].fitness);
                //

                if (alt>=0){
                    std::cout<<"L'avion ne s'est pas crash"<<std::endl;
                }
            }
            // à refaire
            OptiSauvetageGeneral::LogGenerationStats(log_path, i, altitudes_gen, temps_gen, fitness_gen);
            ///

            if (i < nbr_generation - 1) {
                std::vector<OptiSauvetageGeneral::ParamsRescue> population_selected =
                    gen_opti_strat.SortAndKeep(gen_opti_strat.population);
                gen_opti_strat.SaveBestChrom("output/Chromosome_strat_gen_final.txt", population_selected[0]);
                gen_opti_strat.population =
                    gen_opti_strat.Create_Population(nbr_chrom, population_selected);
            } else {
                std::cout << "Taille population finale complète : " << gen_opti_strat.population.size() << std::endl;
                gen_opti_strat.population = gen_opti_strat.SortAndKeep(gen_opti_strat.population);
                std::cout << "Taille meilleur chromosome final : " << gen_opti_strat.population[0].vz_env.size() << std::endl;
                std::cout << "Fitness meilleur finale : " << gen_opti_strat.population[0].fitness << std::endl;
                std::cout << "Taille population finale selectionné : " << gen_opti_strat.population.size() << std::endl;
                gen_opti_strat.SaveBestChrom("output/Chromosome_strat_gen_final.txt", gen_opti_strat.population[0]);
            }
        }

        config.setString("rescue_strategy", Initial_Rescue_Strategy);
        config.setString("quiet_optimizer_logs",Initial_quiet_optimizer_logs);
    }



    
    if (config.hasOperations("SIMULATION")) {
        {
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
    }   

    // Avion avion(361.6, 6.6, 140000.0, true); // hysteresis aerodynamic model
    // avion.initialiser();
    // Simulateur sim(avion, 0.01, 600.0, "simulation_hyst.csv", -0.4, 1.0, 50, 600);
    // sim.executer();

    if (config.hasOperations("ENVELOPPE")) {
        {
        Avion avion_enveloppe(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis")); // linear aerodynamic model // 140000
        AnalyseurEnveloppeVol analyseur(avion_enveloppe);
        analyseur.analyser_limites_vitesse();
        }
    }

    if (config.hasOperations("RUN_BATCH")) {
        run_batch(config.getDouble("p_min"), config.getDouble("p_max"), config.getDouble("p_step"), 
                    config.getDouble("t_min"), config.getDouble("t_max"),  config.getDouble("t_step"), 
                    config.getBool("useHysteresis"), 
                    config.getDouble("dt"), config.getDouble("duree_batch"), 
                    config.getDouble("cmd_start"), config.getDouble("cmd_end"));
    }

    if (config.hasOperations("ENERGIE"))  {
        {
        cout << "ANALYSE ENERGIE (Integration Euler)" << endl;
        main_energie_analysis();
        cout << "ANALYSE ENERGIE (Integration RK4)" << endl;
        main_energie_analysis_rk4();
        }
    }

    // Mettre une boucle ?????????????
    if (config.hasOperations("COMPARE_RESCUE_STRATEGIES"))  { // Mettre une boucle ?????????????
        double crash_time_baseline, crash_time_s0, crash_time_s1, crash_time_s2;
        //Baseline without rescue
        {
        std::cout << "\nBASELINE: NO RESCUE SYSTEM\n" << std::endl;
        Avion avion_no_rescue(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis"));
        avion_no_rescue.initialiser(config.getDouble("vx_ini"), config.getDouble("z_ini"));
        Simulateur sim_no_rescue(avion_no_rescue, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file("output_file/baseline_no_rescue.csv"), 
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
        avion_s0.initialiser(config.getDouble("vx_ini"), config.getDouble("z_ini"));
        Simulateur sim_s0(avion_s0, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file("output_file/strategy0_thrust_first.csv"), 
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
        avion_s1.initialiser(config.getDouble("vx_ini"), config.getDouble("z_ini"));
        Simulateur sim_s1(avion_s1, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file("output_file/strategy1_profile_first.csv"), 
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
        avion_s2.initialiser(config.getDouble("vx_ini"), config.getDouble("z_ini"));
        Simulateur sim_s2(avion_s2, 
                        config.getDouble("dt"), config.getDouble("duree"), 
                        check_output_file("output_file/strategy2_simultaneous.csv"), 
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

    if (config.hasOperations("MIN_RESCUE_ALTITUDE")) {
        std::cout << "\n=== OPERATION MIN_RESCUE_ALTITUDE ===\n" << std::endl;

        const double alt_min = config.getDouble("min_rescue_altitude_min");
        const double alt_max = config.getDouble("min_rescue_altitude_max");
        const double alt_step = std::abs(config.getDouble("min_rescue_altitude_step"));

        std::vector<std::pair<std::string, std::string>> strategies = {
            {"THRUST_FIRST", "strategy0_thrust_first"},
            {"PROFILE_FIRST", "strategy1_profile_first"},
            {"SIMULTANEOUS", "strategy2_simultaneous"}
        };

        std::vector<RescueRunResult> results;

        for (double activation_alt = alt_max; activation_alt >= alt_min - 1e-9; activation_alt -= alt_step) {
            std::cout << "\n[Activation altitude = " << activation_alt << " m]" << std::endl;

            for (const auto& strategy : strategies) {
                config.setString("rescue_strategy", strategy.first);

                Avion avion(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), config.getBool("useHysteresis"));
                avion.initialiser(config.getDouble("vx_ini"), config.getDouble("z_ini"));

                const std::string sim_path = "output/min_rescue_altitude_" + strategy.second + "_a" + std::to_string(static_cast<int>(activation_alt)) + ".csv";
                Simulateur sim(avion,
                               config.getDouble("dt"), config.getDouble("duree"),
                               check_output_file(sim_path),
                               config.getDouble("cmd_profondeur"), config.getDouble("cmd_thrust"),
                               config.getDouble("cmd_start"), config.getDouble("cmd_end"),
                               true,
                               activation_alt);

                double crash_time = sim.executer();
                bool success = std::isnan(crash_time);
                double recovery_altitude = avion.get_altitude();

                results.push_back({activation_alt, strategy.first, success, recovery_altitude, crash_time});

                std::cout << "  - " << strategy.first
                          << " | success=" << (success ? "true" : "false")
                          << " | recovery_altitude=" << std::fixed << std::setprecision(2) << recovery_altitude
                          << " m | crash_time=" << crash_time << std::endl;
            }
        }

        const std::string summary_path = check_output_file("output_file/min_rescue_altitude_results.csv");
        std::ofstream out(summary_path);

        out << "activation_altitude,strategy,success,recovery_altitude,crash_time\n";
        for (const auto& r : results) {
            out << r.activation_altitude << ","
                << r.strategy << ","
                << (r.success ? 1 : 0) << ","
                << r.recovery_altitude << ","
                << r.crash_time << "\n";
        }
        out.close();

        std::cout << "Resultats MIN_RESCUE_ALTITUDE enregistres dans: " << summary_path << std::endl;
    }

    if (config.hasOperations("MIN_RESCUE_ALT_OPT")) {
        std::cout << "\n=== OPERATION MIN_RESCUE_ALT_OPT ===\n" << std::endl;

        const double opt_alt_min  = config.getDouble("min_rescue_altitude_opt_min");
        const double opt_alt_max  = config.getDouble("min_rescue_altitude_opt_max");
        const double opt_alt_step = std::abs(config.getDouble("min_rescue_altitude_opt_step"));
        config.setString("quiet_optimizer_logs", "true");

        const int total_tests = std::max(1, static_cast<int>((opt_alt_max - opt_alt_min) / opt_alt_step + 1.0));
        int test_index = 0;

        OptimiseurSauvetage::ConfigOptimisation opt_cfg;
        opt_cfg.max_iterations   = 5;
        opt_cfg.population_size  = 50;
        opt_cfg.mutation_rate    = 0.15;
        opt_cfg.crossover_rate   = 0.7;
        opt_cfg.verbose          = false;
        OptimiseurSauvetage optimiseur(opt_cfg);

        const std::string opt_summary_path = check_output_file("output_file/min_rescue_altitude_opt_results.csv");
        std::ofstream opt_out(opt_summary_path);
        opt_out << "activation_altitude,best_score,success,recovery_altitude,"
                   "phase_reduction_thrust,phase_reduction_prof,phase_control,"
                   "thrust_reduced_factor,prof_reduced_factor,stabilization_thrust_factor\n";

        for (double act_alt = opt_alt_max; act_alt >= opt_alt_min - 1e-9; act_alt -= opt_alt_step) {
            ++test_index;
            std::cout << "[MIN_RESCUE_ALT_OPT] Alt=" << std::fixed << std::setprecision(0) << act_alt
                      << " m  (" << test_index << "/" << total_tests << ")" << std::endl;

            OptimiseurSauvetage::ConditionsInitiales cond(
                config.getDouble("z_ini"),           // altitude_croisiere
                config.getDouble("vx_ini"),           // vitesse_croisiere
                act_alt,                              // seuil_altitude_critique
                -30.0,                                // seuil_descente_critique
                -0.8,                                 // seuil_pitch_critique
                config.getDouble("cmd_profondeur"),   // cmd_profondeur_perturb
                config.getDouble("cmd_thrust"),        // cmd_thrust_perturb
                config.getDouble("cmd_start"),         // temps_debut_perturb
                config.getDouble("duree")              // duree_simulation
            );

            OptimiseurSauvetage::ParamsRescue best = optimiseur.optimiser_parametres(cond);
            OptimiseurSauvetage::ResultatSimulation res = optimiseur.evaluer_parametres(best, cond);

            std::cout << "    score=" << std::fixed << std::setprecision(3) << res.score
                      << " | success=" << (res.succes_sauvetage ? "true" : "false")
                      << " | alt_finale=" << std::setprecision(2) << res.altitude_finale << " m" << std::endl;

            opt_out << act_alt << ","
                    << res.score << ","
                    << (res.succes_sauvetage ? 1 : 0) << ","
                    << res.altitude_finale << ","
                    << best.phase_reduction_thrust << ","
                    << best.phase_reduction_prof << ","
                    << best.phase_control << ","
                    << best.thrust_reduced_factor << ","
                    << best.prof_reduced_factor << ","
                    << best.stabilization_thrust_factor << "\n";
            opt_out.flush();
        }

        opt_out.close();
        config.setString("quiet_optimizer_logs", "false");
        std::cout << "[OK] Resultats MIN_RESCUE_ALT_OPT enregistres dans: " << opt_summary_path << std::endl;
    }



    return 0;
}