#include "Avion.h"
#include "Simulateur.h"
#include "Constantes.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <limits>

/**
 * @brief Teste une grande plage de valeurs de vitesse et altitude à l'initialisation
 * 
 * Pour chaque combinaison (vitesse, altitude):
 * - Initialise l'avion avec ces paramètres
 * - Exécute une simulation complète (600s par défaut)
 * - Enregistre les valeurs maximales de alpha et pitch atteintes
 * - Vérifie la convergence du calcul de trim
 * 
 * @param vx_min Vitesse minimale (m/s)
 * @param vx_max Vitesse maximale (m/s)
 * @param vx_step Pas de vitesse (m/s)
 * @param z_min Altitude minimale (m)
 * @param z_max Altitude maximale (m)
 * @param z_step Pas d'altitude (m)
 * @param useHysteresis Utiliser le modèle avec hystérésis
 * @param sim_duration Durée de simulation pour chaque test (s)
 * @param output_file Nom du fichier de sortie
 */
int test_initialization_range(
    double vx_min, double vx_max, double vx_step,
    double z_min, double z_max, double z_step,
    bool useHysteresis = false,
    double sim_duration = 600.0,
    const std::string& output_file = "output/init_range_test.csv"
) {
    using namespace Physique;
    
    // Vérification des paramètres
    if (vx_step <= 0.0 || z_step <= 0.0) {
        std::cerr << "Erreur: les pas doivent être positifs" << std::endl;
        return 1;
    }
    
    // Création des vecteurs de test
    std::vector<double> vitesses;
    std::vector<double> altitudes;
    
    for (double v = vx_min; v <= vx_max + 1e-12; v += vx_step) {
        vitesses.push_back(v);
    }
    for (double z = z_min; z <= z_max + 1e-12; z += z_step) {
        altitudes.push_back(z);
    }
    
    std::cout << "Test de " << vitesses.size() << " vitesses x " 
              << altitudes.size() << " altitudes = " 
              << (vitesses.size() * altitudes.size()) << " combinaisons\n" << std::endl;
    
    // Ouverture du fichier de sortie
    std::ofstream out(output_file);
    if (!out.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir " << output_file << std::endl;
        return 1;
    }
    
    // En-tête du CSV
    out << "vitesse_init,altitude_init,trim_converged,alpha_trim,delta_p_trim,"
        << "alpha_max,pitch_max,crash,crash_time\n";
    out << std::fixed << std::setprecision(6);
    
    // Boucle sur toutes les combinaisons
    int test_count = 0;
    int convergence_failures = 0;
    
    for (double vx : vitesses) {
        for (double z : altitudes) {
            test_count++;
            std::cout << "\n[Test " << test_count << "/" << (vitesses.size() * altitudes.size()) 
                      << "] Vitesse=" << vx << " m/s, Altitude=" << z << " m" << std::endl;
            
            // Créer un nouvel avion pour chaque test
            Avion avion(361.6, 6.6, 205000.0, useHysteresis);
            
            // Initialiser avec les paramètres de test
            avion.initialiser(vx, z);
            
            // Calculer le trim complet et vérifier la convergence
            bool trim_converged = true;
            double alpha_trim = 0.0;
            double delta_p_trim = 0.0;
            
            try {
                auto trim_result = avion.calculer_trim_complet(vx);
                alpha_trim = trim_result.first;
                delta_p_trim = trim_result.second;
                
                // Vérifier la convergence en calculant les erreurs
                avion.get_aero().update_from_polar(
                    alpha_trim, delta_p_trim, 0.0, vx,
                    avion.get_env().calculer_mach(vx, z)
                );
                
                double rho = avion.get_env().calculer_rho(z);
                double L = avion.get_aero().calculer_portance(vx, rho);
                double W = avion.get_masse() * g;
                double erreur_L = std::fabs(L - W);
                
                double D = avion.get_aero().calculer_trainee(vx, rho);
                double M_aero = avion.get_aero().calculer_moment_pitch(vx, rho);
                double M_thrust = Physique::z_t * D;
                double erreur_M = std::fabs(M_aero + M_thrust);
                
                // Critères de convergence
                if (erreur_L > 1000.0 || erreur_M > 50000.0) {
                    trim_converged = false;
                    convergence_failures++;
                    std::cout << "  TRIM DIVERGENCE: erreur_L=" << erreur_L 
                              << " N, erreur_M=" << erreur_M << " N.m" << std::endl;
                } else {
                    std::cout << "  Trim OK: alpha=" << (alpha_trim * RAD_TO_DEG) 
                              << "°, delta_p=" << delta_p_trim << " rad" << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "  Exception lors du calcul de trim: " << e.what() << std::endl;
                trim_converged = false;
                convergence_failures++;
            }
            
            // Créer un nom de fichier pour la simulation
            std::ostringstream temp_file;
            temp_file << "output/sim_init_v" << static_cast<int>(vx) 
                      << "_z" << static_cast<int>(z) << ".csv";
            
            // Simuler avec les mêmes paramètres que la simulation principale
            Simulateur sim(avion, 0.01, sim_duration, temp_file.str(), 
                          -0.8, 1.0,  // cmd_profondeur=-0.8, cmd_thrust=1.0
                          100.0, 600.0);  // cmd_start=100, cmd_end=600
            
            double crash_time = sim.executer();
            bool crashed = !std::isnan(crash_time);
            
            // Analyser les résultats pour trouver alpha_max et pitch_max
            double alpha_max = 0.0;
            double pitch_max = 0.0;
            
            std::ifstream sim_file(temp_file.str());
            if (sim_file.is_open()) {
                std::string line;
                std::getline(sim_file, line);  // Skip header
                
                while (std::getline(sim_file, line)) {
                    if (line.empty()) continue;
                    
                    std::stringstream ss(line);
                    std::string item;
                    std::vector<std::string> tokens;
                    
                    while (std::getline(ss, item, ',')) {
                        tokens.push_back(item);
                    }
                    
                    // Format: time, x, y, z, vx, vy, vz, roll, pitch, yaw, omega_pitch
                    // Alpha est calculé dans le simulateur et écrit après omega_pitch
                    // On suppose que alpha et pitch sont dans les dernières colonnes
                    if (tokens.size() >= 9) {
                        try {
                            double pitch = std::stod(tokens[8]);  // pitch (rad)
                            pitch_max = std::max(pitch_max, std::fabs(pitch));
                            
                            // Si alpha est disponible dans le fichier (après omega_pitch)
                            if (tokens.size() >= 12) {
                                double alpha = std::stod(tokens[11]);
                                alpha_max = std::max(alpha_max, std::fabs(alpha));
                            }
                        } catch (...) {
                            // Ignorer les erreurs de parsing
                        }
                    }
                }
                sim_file.close();
            }
            
            // Si alpha n'est pas dans le fichier, utiliser alpha_trim comme approximation
            if (alpha_max == 0.0) {
                alpha_max = std::fabs(alpha_trim);
            }
            
            // Écrire les résultats
            out << vx << "," << z << "," 
                << (trim_converged ? 1 : 0) << ","
                << (alpha_trim * RAD_TO_DEG) << ","  // Convertir en degrés
                << delta_p_trim << ","
                << (alpha_max * RAD_TO_DEG) << ","   // Convertir en degrés
                << (pitch_max * RAD_TO_DEG) << ","   // Convertir en degrés
                << (crashed ? 1 : 0) << ",";
            
            if (crashed) {
                out << crash_time;
            }
            out << "\n";
            out.flush();
            
            std::cout << "  Alpha max: " << (alpha_max * RAD_TO_DEG) 
                      << "°, Pitch max: " << (pitch_max * RAD_TO_DEG) << "°";
            if (crashed) {
                std::cout << ", CRASH à t=" << crash_time << "s";
            }
            std::cout << std::endl;
        }
    }
    
    out.close();
    
    // Résumé
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test terminé: " << test_count << " combinaisons testées" << std::endl;
    std::cout << "Échecs de convergence du trim: " << convergence_failures 
              << " (" << (100.0 * convergence_failures / test_count) << "%)" << std::endl;
    std::cout << "Résultats sauvegardés dans: " << output_file << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
