#ifndef OPTIMISEUR_SAUVETAGE_H
#define OPTIMISEUR_SAUVETAGE_H

#include <vector>
#include <utility>
#include <string>

class Avion;

/**
 * Machine learning-based optimizer for rescue system parameters
 * Uses adaptive search to find optimal rescue parameters for given conditions
 */
class OptimiseurSauvetage {
public:
    // Rescue parameters to optimize
    struct ParamsRescue {
        double phase_reduction_thrust;  // Duration of thrust reduction phase (s)
        double phase_reduction_prof;    // Duration of elevator reduction phase (s)
        double phase_control;           // Duration of control stabilization phase (s)
        double thrust_reduced_factor;   // Thrust reduction factor [0, 1]
        double prof_reduced_factor;     // Elevator reduction factor [-1, 1]
        double stabilization_thrust_factor; // Final thrust target ratio after rescue [0, 1]
        
        ParamsRescue() 
            : phase_reduction_thrust(0.6),
              phase_reduction_prof(0.6),
              phase_control(3.0),
              thrust_reduced_factor(0.2),
              prof_reduced_factor(-0.2),
              stabilization_thrust_factor(0.6) {}
        
        ParamsRescue(double prt, double prp, double pc, double trf, double prf, double stf = 0.6)
            : phase_reduction_thrust(prt),
              phase_reduction_prof(prp),
              phase_control(pc),
              thrust_reduced_factor(trf),
              prof_reduced_factor(prf),
              stabilization_thrust_factor(stf) {}
    };
    
    // Initial conditions for rescue scenario
    struct ConditionsInitiales {
        // Cruise flight parameters
        double altitude_croisiere;      // Cruise altitude (m)
        double vitesse_croisiere;       // Cruise speed (m/s)
        
        // Critical thresholds for rescue activation
        double seuil_altitude_critique;   // Critical altitude threshold (m)
        double seuil_descente_critique;   // Critical descent rate threshold (m/s)
        double seuil_pitch_critique;      // Critical pitch threshold (rad)
        
        // Perturbation commands to provoke descent
        double cmd_profondeur_perturb;  // Elevator command for perturbation
        double cmd_thrust_perturb;      // Thrust command for perturbation
        double temps_debut_perturb;     // Time to start perturbation (s)
        double duree_simulation;        // Simulation duration (s)
        
        ConditionsInitiales(double alt_cruise = 10670.0, double v_cruise = 240.0,
                   double seuil_alt = 3000.0, double seuil_d = -30.0, double seuil_p = -0.8,
                   double cmd_prof = -0.4, double cmd_thr = 1.0,
                   double t_perturb = 50.0, double duree = 600.0)
            : altitude_croisiere(alt_cruise), vitesse_croisiere(v_cruise),
              seuil_altitude_critique(seuil_alt),
              seuil_descente_critique(seuil_d), seuil_pitch_critique(seuil_p),
              cmd_profondeur_perturb(cmd_prof), cmd_thrust_perturb(cmd_thr),
              temps_debut_perturb(t_perturb), duree_simulation(duree) {}
    };
    
    // Simulation result for evaluation
    struct ResultatSimulation {
        bool succes_sauvetage;      // Did rescue succeed?
        double altitude_finale;     // Final altitude (m)
        double temps_recuperation;  // Time to recover (s)
        double vitesse_finale;      // Final speed (m/s)
        double max_load_factor;     // Maximum load factor during recovery
        double score;               // Overall score for optimization
        
        ResultatSimulation() 
            : succes_sauvetage(false), altitude_finale(0.0),
              temps_recuperation(999.0), vitesse_finale(0.0),
              max_load_factor(0.0), score(0.0) {}
    };
    
    // Configuration for optimization algorithm
    struct ConfigOptimisation {
        int max_iterations;         // Maximum iterations
        int population_size;        // Population size for genetic algorithm
        double mutation_rate;       // Mutation probability
        double crossover_rate;      // Crossover probability
        double convergence_tol;     // Convergence tolerance
        bool verbose;               // Print progress
        
        ConfigOptimisation()
            : max_iterations(50),
              population_size(20),
              mutation_rate(0.15),
              crossover_rate(0.7),
              convergence_tol(0.001),
              verbose(true) {}
    };

    OptimiseurSauvetage(const ConfigOptimisation& config = ConfigOptimisation());
    
    /**
     * Find optimal rescue parameters for given initial conditions
     * @param conditions Initial flight conditions requiring rescue
     * @return Optimized rescue parameters
     */
    ParamsRescue optimiser_parametres(const ConditionsInitiales& conditions);
    
    /**
     * Evaluate a set of parameters for given conditions
     * @param params Parameters to evaluate
     * @param conditions Initial conditions
     * @return Simulation result with score
     */
    ResultatSimulation evaluer_parametres(const ParamsRescue& params,
                                         const ConditionsInitiales& conditions);
    
    /**
     * Save/load optimized parameters database
     */
    void sauvegarder_database(const std::string& filename);
    void charger_database(const std::string& filename);
    
    /**
     * Query optimized parameters for similar conditions
     * Uses nearest neighbor interpolation from database
     */
    ParamsRescue interroger_database(const ConditionsInitiales& conditions);
    
private:
    ConfigOptimisation config_;
    
    // Database of optimized parameters
    struct EntreeDatabase {
        ConditionsInitiales conditions;
        ParamsRescue params;
        double score;
    };
    std::vector<EntreeDatabase> database_;
    
    // Genetic algorithm components
    std::vector<ParamsRescue> initialiser_population(const ConditionsInitiales& conditions);
    ParamsRescue mutate(const ParamsRescue& params);
    ParamsRescue crossover(const ParamsRescue& parent1, const ParamsRescue& parent2);
    std::vector<ParamsRescue> selection(const std::vector<ParamsRescue>& population,
                                       const std::vector<double>& scores);
    
    // Parameter bounds
    void clamp_parameters(ParamsRescue& params);
    
    // Scoring function
    double calculer_score(const ResultatSimulation& resultat);
    
    // Simulation wrapper
    ResultatSimulation simuler_avec_params(const ParamsRescue& params,
                                          const ConditionsInitiales& conditions);
    
    // Distance metric for database queries
    double distance_conditions(const ConditionsInitiales& c1, 
                              const ConditionsInitiales& c2);
};

#endif // OPTIMISEUR_SAUVETAGE_H
