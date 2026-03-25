#include "OptimiseurSauvetage.h"
#include "Avion.h"
#include "Simulateur.h"
#include "SauvetageAvion.h"
#include "Constantes.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <limits>
#include <iomanip>

namespace {
bool optimizer_quiet_mode() {
    try {
        return config.getBool("quiet_optimizer_logs");
    } catch (...) {
        return false;
    }
}
}

OptimiseurSauvetage::OptimiseurSauvetage(const ConfigOptimisation& config)
    : config_(config) {}

OptimiseurSauvetage::ParamsRescue OptimiseurSauvetage::optimiser_parametres(
    const ConditionsInitiales& conditions) {
    
    if (config_.verbose) {
        std::cout << "\n========== OPTIMISATION PARAMETRES SAUVETAGE ==========" << std::endl;
        std::cout << "Conditions initiales:" << std::endl;
        std::cout << "  Croisiere: Alt=" << conditions.altitude_croisiere
                  << " m, V=" << conditions.vitesse_croisiere << " m/s" << std::endl;
        std::cout << "  Seuils: Alt<" << conditions.seuil_altitude_critique
                  << " m ET Vz<" << conditions.seuil_descente_critique
                  << " m/s (pitch<" << (conditions.seuil_pitch_critique * Math::RAD_TO_DEG) << " deg)" << std::endl;
        std::cout << "  Perturbation: cmd_prof=" << conditions.cmd_profondeur_perturb
                  << ", cmd_thrust=" << conditions.cmd_thrust_perturb
                  << ", t0=" << conditions.temps_debut_perturb << " s" << std::endl;
        std::cout << "======================================================\n" << std::endl;
    }
    
    // Initialize population
    std::vector<ParamsRescue> population = initialiser_population(conditions);
    std::vector<double> scores(population.size());
    
    ParamsRescue best_params;
    double best_score = -std::numeric_limits<double>::infinity();
    int generations_without_improvement = 0;
    
    // Genetic algorithm main loop
    for (int iter = 0; iter < config_.max_iterations; ++iter) {
        if (optimizer_quiet_mode()) {
            const int bar_width = 28;
            const double progress = static_cast<double>(iter + 1) / static_cast<double>(std::max(1, config_.max_iterations));
            const int filled = static_cast<int>(progress * bar_width);
            std::string bar(filled, '#');
            bar += std::string(bar_width - filled, '-');
            std::cout << "\r  Alt=" << std::fixed << std::setprecision(0) << conditions.seuil_altitude_critique
                      << " m | optimisation [" << bar << "] "
                      << std::setw(3) << static_cast<int>(progress * 100.0) << "%" << std::flush;
        }

        // Evaluate all individuals
        for (size_t i = 0; i < population.size(); ++i) {
            ResultatSimulation resultat = evaluer_parametres(population[i], conditions);
            scores[i] = resultat.score;
            
            // Track best
            if (scores[i] > best_score) {
                best_score = scores[i];
                best_params = population[i];
                generations_without_improvement = 0;
                
                if (config_.verbose) {
                    std::cout << "[Iter " << iter << "] Nouveau meilleur score: " << best_score
                              << " | Succes: " << (resultat.succes_sauvetage ? "OUI" : "NON")
                              << " | Alt finale: " << resultat.altitude_finale << " m" << std::endl;
                }
            }
        }
        
        generations_without_improvement++;
        
        // Check convergence
        if (generations_without_improvement > 10 && best_score > 0.8) {
            if (config_.verbose) {
                std::cout << "\nConvergence atteinte apres " << iter << " iterations." << std::endl;
            }
            break;
        }
        
        // Selection
        std::vector<ParamsRescue> selected = selection(population, scores);
        
        // Create new generation
        std::vector<ParamsRescue> new_population;
        new_population.push_back(best_params); // Elitism: keep best
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        while (new_population.size() < population.size()) {
            // Select parents
            int idx1 = gen() % selected.size();
            int idx2 = gen() % selected.size();
            
            ParamsRescue offspring;
            
            // Crossover
            if (dis(gen) < config_.crossover_rate) {
                offspring = crossover(selected[idx1], selected[idx2]);
            } else {
                offspring = selected[idx1];
            }
            
            // Mutation
            if (dis(gen) < config_.mutation_rate) {
                offspring = mutate(offspring);
            }
            
            clamp_parameters(offspring);
            new_population.push_back(offspring);
        }
        
        population = new_population;
    }

    if (optimizer_quiet_mode()) {
        std::cout << std::endl;
    }
    
    if (config_.verbose) {
        std::cout << "\n========== OPTIMISATION TERMINEE ==========" << std::endl;
        std::cout << "Meilleur score: " << best_score << std::endl;
        std::cout << "Parametres optimaux:" << std::endl;
        std::cout << "  Phase reduction thrust: " << best_params.phase_reduction_thrust << " s" << std::endl;
        std::cout << "  Phase reduction prof: " << best_params.phase_reduction_prof << " s" << std::endl;
        std::cout << "  Phase control: " << best_params.phase_control << " s" << std::endl;
        std::cout << "  Thrust reduced factor: " << best_params.thrust_reduced_factor << std::endl;
        std::cout << "  Prof reduced factor: " << best_params.prof_reduced_factor << std::endl;
        std::cout << "==========================================\n" << std::endl;
    }
    
    // Add to database
    EntreeDatabase entree;
    entree.conditions = conditions;
    entree.params = best_params;
    entree.score = best_score;
    database_.push_back(entree);
    
    return best_params;
}

OptimiseurSauvetage::ResultatSimulation OptimiseurSauvetage::evaluer_parametres(
    const ParamsRescue& params, const ConditionsInitiales& conditions) {
    
    return simuler_avec_params(params, conditions);
}

OptimiseurSauvetage::ResultatSimulation OptimiseurSauvetage::simuler_avec_params(
    const ParamsRescue& params, const ConditionsInitiales& conditions) {
    
    ResultatSimulation resultat;
    
    auto params_anciens = SauvetageAvion::get_parametres();
    auto seuils_anciens = SauvetageAvion::get_seuils_critiques();

    try {
        static bool config_initialisee = false;
        if (!config_initialisee) {
            try {
                config.chargerDepuisFichier("Config.txt");
                config.completer();
            } catch (...) {
                config.setString("operations", "SIMULATION");
                config.setString("g", "9.81");
                config.setString("z_t", "2.0");
                config.setString("methode_integration", "RK4");
                config.setString("rescue_strategy", "THRUST_FIRST");
            }
            config_initialisee = true;
        }

        // Force PROFILE_FIRST strategy for this optimizer
        config.setString("rescue_strategy", "PROFILE_FIRST");

        SauvetageAvion::Parametres params_dyn;
        params_dyn.phase_reduction_thrust = params.phase_reduction_thrust;
        params_dyn.phase_reduction_prof = params.phase_reduction_prof;
        params_dyn.phase_control = params.phase_control;
        params_dyn.thrust_reduced_factor = params.thrust_reduced_factor;
        params_dyn.prof_reduced_factor = params.prof_reduced_factor;
        params_dyn.stabilization_thrust_factor = params.stabilization_thrust_factor;
        SauvetageAvion::set_parametres(params_dyn);

        SauvetageAvion::set_seuils_critiques(
            conditions.seuil_descente_critique,
            conditions.seuil_pitch_critique);

        // Create aircraft with initial conditions
        Avion avion(config.getDouble("surface"), config.getDouble("corde"), config.getDouble("masse"), false); // linear aerodynamic model // 140000

        avion.initialiser();
        
        // Set initial cruise state - horizontal flight
        avion.get_etat().z = conditions.altitude_croisiere;
        avion.get_etat().vx = conditions.vitesse_croisiere;
        avion.get_etat().vy = 0.0;
        avion.get_etat().vz = 0.0;  // Start with level flight
        
        // Compute trim for cruise conditions
        double speed_cruise = conditions.vitesse_croisiere;
        std::pair<double, double> trim_result = avion.calculer_trim_complet(speed_cruise);
        double alpha_trim = trim_result.first;
        double delta_trim = trim_result.second;
        avion.get_etat().pitch = alpha_trim;
        avion.get_aero().set_delta_profondeur(delta_trim);
        
        // Run simulation with rescue enabled
        // Perturbation commands will be applied at temps_debut_perturb
        std::string temp_file = "temp_rescue_eval.csv";
        Simulateur sim(avion, 
                  0.01,                                    // dt
                  conditions.duree_simulation,             // duree
                  temp_file,                               // fichier
                  conditions.cmd_profondeur_perturb,       // cmd_profondeur 
                  conditions.cmd_thrust_perturb,           // cmd_thrust
                  conditions.temps_debut_perturb,          // cmd_start
                  conditions.duree_simulation,             // cmd_end
                  true,                                    // enable_rescue
                  conditions.seuil_altitude_critique);      // seuil_altitude

        double crash_time = sim.executer();
        
        // Retrieve recovery info from simulator (if any)
        double sim_recovery_time = sim.get_dernier_temps_recuperation();
        double sim_recovery_altitude = sim.get_derniere_altitude_recuperation();

        // Evaluate results
        // altitude_finale: prefer altitude at moment of recovery if available
        if (sim_recovery_time < 999.0) {
            resultat.altitude_finale = sim_recovery_altitude;
        } else {
            resultat.altitude_finale = avion.get_altitude();
        }
        resultat.vitesse_finale = avion.get_etat().get_vitesse_norme();
        resultat.max_load_factor = 0.0;

        std::ifstream csv(temp_file);
        std::string line;
        if (csv.is_open()) {
            std::getline(csv, line);
            while (std::getline(csv, line)) {
                std::stringstream ss(line);
                std::string token;
                std::vector<std::string> cols;
                while (std::getline(ss, token, ',')) {
                    cols.push_back(token);
                }
                if (cols.size() > 29) {
                    try {
                        double n = std::stod(cols[29]);
                        resultat.max_load_factor = std::max(resultat.max_load_factor, std::fabs(n));
                    } catch (...) {
                    }
                }
            }
            csv.close();
        }
        
        // Check if rescue succeeded
        if (std::isnan(crash_time) && avion.get_altitude() > 100.0 && avion.get_vitesse_z() > -5.0) {
            resultat.succes_sauvetage = true;
            // If simulator recorded a recovery time, use it; otherwise fallback to simulation duration
            if (sim_recovery_time < 999.0) {
                resultat.temps_recuperation = sim_recovery_time;
            } else {
                resultat.temps_recuperation = conditions.duree_simulation;
            }
        } else {
            resultat.succes_sauvetage = false;
            resultat.temps_recuperation = 999.0;
        }
        
        // Calculate score
        resultat.score = calculer_score(resultat);
        
    } catch (const std::exception& e) {
        // Simulation failed
        resultat.succes_sauvetage = false;
        resultat.score = -1000.0;
        if (config_.verbose) {
            std::cerr << "[Optimiseur] Erreur simulation: " << e.what() << std::endl;
        }
    } catch (...) {
        resultat.succes_sauvetage = false;
        resultat.score = -1000.0;
    }

    SauvetageAvion::set_parametres(params_anciens);
    SauvetageAvion::set_seuils_critiques(seuils_anciens.first, seuils_anciens.second);
    
    return resultat;
}

double OptimiseurSauvetage::calculer_score(const ResultatSimulation& resultat) {
    double score = 0.0;
    
    // Primary: did rescue succeed?
    if (resultat.succes_sauvetage) {
        score += 100.0;
    } else {
        score -= 100.0;
    }
    
    // Secondary: altitude gained/preserved
    score += resultat.altitude_finale * 0.01;  // 0.01 points per meter
    
    // Tertiary: speed in safe range
    if (resultat.vitesse_finale > 120.0 && resultat.vitesse_finale < 350.0) {
        score += 20.0;
    } else {
        score -= std::abs(resultat.vitesse_finale - 200.0) * 0.1;
    }
    
    // Penalty for long recovery time
    if (resultat.temps_recuperation < 10.0) {
        score += (10.0 - resultat.temps_recuperation) * 2.0;
    }
    
    // Penalty for high load factor
    if (resultat.max_load_factor > 2.5) {
        score -= (resultat.max_load_factor - 2.5) * 10.0;
    }
    
    return score;
}

std::vector<OptimiseurSauvetage::ParamsRescue> 
OptimiseurSauvetage::initialiser_population(const ConditionsInitiales& /* conditions */) {
    
    std::vector<ParamsRescue> population;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Add default parameters
    population.push_back(ParamsRescue());
    
    // Generate random variations
    std::uniform_real_distribution<> phase_rt_dist(0.1, 10.0);
    std::uniform_real_distribution<> phase_rp_dist(0.1, 10.0);
    std::uniform_real_distribution<> phase_c_dist(0.5, 25.0);
    std::uniform_real_distribution<> thrust_f_dist(0.0, 1.0);
    std::uniform_real_distribution<> prof_f_dist(-1.0, 1.0);
    std::uniform_real_distribution<> stab_f_dist(0.0, 1.0);
    
    for (int i = 1; i < config_.population_size; ++i) {
        ParamsRescue params;
        params.phase_reduction_thrust = phase_rt_dist(gen);
        params.phase_reduction_prof = phase_rp_dist(gen);
        params.phase_control = phase_c_dist(gen);
        params.thrust_reduced_factor = thrust_f_dist(gen);
        params.prof_reduced_factor = prof_f_dist(gen);
        params.stabilization_thrust_factor = stab_f_dist(gen);
        
        population.push_back(params);
    }
    
    return population;
}

OptimiseurSauvetage::ParamsRescue OptimiseurSauvetage::mutate(const ParamsRescue& params) {
    ParamsRescue mutated = params;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> phase_noise(0.0, 1.0);
    std::normal_distribution<> factor_noise(0.0, 0.2);
    
    mutated.phase_reduction_thrust += phase_noise(gen);
    mutated.phase_reduction_prof += phase_noise(gen);
    mutated.phase_control += phase_noise(gen) * 0.5;
    mutated.thrust_reduced_factor += factor_noise(gen);
    mutated.prof_reduced_factor += factor_noise(gen);
    mutated.stabilization_thrust_factor += factor_noise(gen);
    
    clamp_parameters(mutated);
    return mutated;
}

OptimiseurSauvetage::ParamsRescue OptimiseurSauvetage::crossover(
    const ParamsRescue& parent1, const ParamsRescue& parent2) {
    
    ParamsRescue offspring;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    // Uniform crossover
    offspring.phase_reduction_thrust = (dis(gen) < 0.5) ? 
        parent1.phase_reduction_thrust : parent2.phase_reduction_thrust;
    offspring.phase_reduction_prof = (dis(gen) < 0.5) ? 
        parent1.phase_reduction_prof : parent2.phase_reduction_prof;
    offspring.phase_control = (dis(gen) < 0.5) ? 
        parent1.phase_control : parent2.phase_control;
    offspring.thrust_reduced_factor = (dis(gen) < 0.5) ? 
        parent1.thrust_reduced_factor : parent2.thrust_reduced_factor;
    offspring.prof_reduced_factor = (dis(gen) < 0.5) ? 
        parent1.prof_reduced_factor : parent2.prof_reduced_factor;
    offspring.stabilization_thrust_factor = (dis(gen) < 0.5) ?
        parent1.stabilization_thrust_factor : parent2.stabilization_thrust_factor;
    
    return offspring;
}

std::vector<OptimiseurSauvetage::ParamsRescue> OptimiseurSauvetage::selection(
    const std::vector<ParamsRescue>& population, const std::vector<double>& scores) {
    
    std::vector<ParamsRescue> selected;
    std::vector<std::pair<double, int>> scored_indices;
    
    for (size_t i = 0; i < scores.size(); ++i) {
        scored_indices.push_back({scores[i], i});
    }
    
    // Sort by score (descending)
    std::sort(scored_indices.begin(), scored_indices.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Take top 50%
    int num_selected = population.size() / 2;
    for (int i = 0; i < num_selected; ++i) {
        selected.push_back(population[scored_indices[i].second]);
    }
    
    return selected;
}

void OptimiseurSauvetage::clamp_parameters(ParamsRescue& params) {
    // Phase durations: [0.1, 10.0] / [0.5, 25.0] seconds
    params.phase_reduction_thrust = std::max(0.1, std::min(10.0, params.phase_reduction_thrust));
    params.phase_reduction_prof = std::max(0.1, std::min(10.0, params.phase_reduction_prof));
    params.phase_control = std::max(0.5, std::min(25.0, params.phase_control));
    
    // Reduction factors
    params.thrust_reduced_factor = std::max(0.0, std::min(1.0, params.thrust_reduced_factor));
    params.prof_reduced_factor = std::max(-1.0, std::min(1.0, params.prof_reduced_factor));
    params.stabilization_thrust_factor = std::max(0.0, std::min(1.0, params.stabilization_thrust_factor));
}

void OptimiseurSauvetage::sauvegarder_database(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir " << filename << std::endl;
        return;
    }
    
    file << "alt_cruise,v_cruise,seuil_alt,seuil_descente,seuil_pitch,";
    file << "cmd_prof_perturb,cmd_thrust_perturb,t_debut_perturb,duree_sim,";
    file << "phase_rt,phase_rp,phase_c,thrust_f,prof_f,score\n";
    
    for (const auto& entree : database_) {
        file << entree.conditions.altitude_croisiere << ","
             << entree.conditions.vitesse_croisiere << ","
             << entree.conditions.seuil_altitude_critique << ","
             << entree.conditions.seuil_descente_critique << ","
             << entree.conditions.seuil_pitch_critique << ","
             << entree.conditions.cmd_profondeur_perturb << ","
             << entree.conditions.cmd_thrust_perturb << ","
             << entree.conditions.temps_debut_perturb << ","
             << entree.conditions.duree_simulation << ","
             << entree.params.phase_reduction_thrust << ","
             << entree.params.phase_reduction_prof << ","
             << entree.params.phase_control << ","
             << entree.params.thrust_reduced_factor << ","
             << entree.params.prof_reduced_factor << ","
             << entree.score << "\n";
    }
    
    file.close();
    std::cout << "Database sauvegardee: " << database_.size() << " entrees dans " 
              << filename << std::endl;
}

void OptimiseurSauvetage::charger_database(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir " << filename << std::endl;
        return;
    }
    
    database_.clear();
    std::string line;
    std::getline(file, line); // Skip header
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        EntreeDatabase entree;
        
        std::vector<double> values;
        std::string token;
        while (std::getline(ss, token, ',')) {
            try {
                values.push_back(std::stod(token));
            } catch (...) {
            }
        }

        if (values.size() >= 15) {
            // New format: alt_cruise,v_cruise,seuil_alt,seuil_descente,seuil_pitch,
            //             cmd_prof_perturb,cmd_thrust_perturb,t_debut_perturb,duree_sim,
            //             phase_rt,phase_rp,phase_c,thrust_f,prof_f,score
            entree.conditions.altitude_croisiere = values[0];
            entree.conditions.vitesse_croisiere = values[1];
            entree.conditions.seuil_altitude_critique = values[2];
            entree.conditions.seuil_descente_critique = values[3];
            entree.conditions.seuil_pitch_critique = values[4];
            entree.conditions.cmd_profondeur_perturb = values[5];
            entree.conditions.cmd_thrust_perturb = values[6];
            entree.conditions.temps_debut_perturb = values[7];
            entree.conditions.duree_simulation = values[8];
            entree.params.phase_reduction_thrust = values[9];
            entree.params.phase_reduction_prof = values[10];
            entree.params.phase_control = values[11];
            entree.params.thrust_reduced_factor = values[12];
            entree.params.prof_reduced_factor = values[13];
            entree.score = values[14];
        } else if (values.size() >= 13) {
            // Old format with 13 columns - convert to new format with defaults
            entree.conditions.altitude_croisiere = 10670.0;
            entree.conditions.vitesse_croisiere = 240.0;
            entree.conditions.seuil_altitude_critique = 3000.0;
            entree.conditions.seuil_descente_critique = values[5];
            entree.conditions.seuil_pitch_critique = values[6];
            entree.conditions.cmd_profondeur_perturb = -0.4;
            entree.conditions.cmd_thrust_perturb = 1.0;
            entree.conditions.temps_debut_perturb = 50.0;
            entree.conditions.duree_simulation = 600.0;
            entree.params.phase_reduction_thrust = values[7];
            entree.params.phase_reduction_prof = values[8];
            entree.params.phase_control = values[9];
            entree.params.thrust_reduced_factor = values[10];
            entree.params.prof_reduced_factor = values[11];
            entree.score = values[12];
        } else {
            continue;
        }
        
        database_.push_back(entree);
    }
    
    file.close();
    std::cout << "Database chargee: " << database_.size() << " entrees depuis " 
              << filename << std::endl;
}

OptimiseurSauvetage::ParamsRescue OptimiseurSauvetage::interroger_database(
    const ConditionsInitiales& conditions) {
    
    if (database_.empty()) {
        std::cerr << "Warning: database vide, retour parametres par defaut" << std::endl;
        return ParamsRescue();
    }
    
    // Find nearest neighbor
    double min_distance = std::numeric_limits<double>::infinity();
    int best_idx = 0;
    
    for (size_t i = 0; i < database_.size(); ++i) {
        double dist = distance_conditions(conditions, database_[i].conditions);
        if (dist < min_distance) {
            min_distance = dist;
            best_idx = i;
        }
    }
    
    if (config_.verbose) {
        std::cout << "Parametres trouves dans database (distance: " 
                  << min_distance << ")" << std::endl;
    }
    
    return database_[best_idx].params;
}

double OptimiseurSauvetage::distance_conditions(
    const ConditionsInitiales& c1, const ConditionsInitiales& c2) {
    // Weighted Euclidean distance based on scenario parameters
    // Focus on thresholds and perturbation severity rather than initial cruise state
    double d_seuil_alt = (c1.seuil_altitude_critique - c2.seuil_altitude_critique) / 2000.0;
    double d_seuil_desc = (c1.seuil_descente_critique - c2.seuil_descente_critique) / 20.0;
    double d_seuil_pitch = (c1.seuil_pitch_critique - c2.seuil_pitch_critique) / 0.3;
    double d_cmd_prof = (c1.cmd_profondeur_perturb - c2.cmd_profondeur_perturb) / 0.5;
    double d_cmd_thrust = (c1.cmd_thrust_perturb - c2.cmd_thrust_perturb) / 0.5;
    
    // Also include cruise parameters with lower weight
    double d_alt_cruise = (c1.altitude_croisiere - c2.altitude_croisiere) / 5000.0;
    double d_v_cruise = (c1.vitesse_croisiere - c2.vitesse_croisiere) / 100.0;
    
    return std::sqrt(d_seuil_alt * d_seuil_alt + 
                    d_seuil_desc * d_seuil_desc + 
                    d_seuil_pitch * d_seuil_pitch +
                    d_cmd_prof * d_cmd_prof +
                    d_cmd_thrust * d_cmd_thrust +
                    0.2 * d_alt_cruise * d_alt_cruise +
                    0.2 * d_v_cruise * d_v_cruise);
}
