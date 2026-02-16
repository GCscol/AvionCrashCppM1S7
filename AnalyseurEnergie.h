#ifndef ANALYSEUR_ENERGIE_H
#define ANALYSEUR_ENERGIE_H

#include <string>
#include "Avion.h"

/**
 * @brief Analyse l'évolution de l'énergie du système
 * Calcule: Énergie cinétique, Énergie potentielle, Travail des forces
 */
int analyser_energie_simulation(const std::string& fichier_csv_sortie,
                                double pas_temps,
                                double duree_sim,
                                double commande_profondeur,
                                double commande_thrust,
                                double cmd_start,
                                double cmd_end,
                                bool use_hysteresis);

/**
 * @brief Analyse l'énergie avec intégration RK4 (Runge-Kutta 4ème ordre)
 * Beaucoup plus précis que la méthode par défaut
 */
int analyser_energie_simulation_rk4(const std::string& fichier_csv_sortie,
                                    double pas_temps,
                                    double duree_sim,
                                    double commande_profondeur,
                                    double commande_thrust,
                                    double cmd_start,
                                    double cmd_end,
                                    bool use_hysteresis);

int main_energie_analysis();

/**
 * @brief Lance l'analyse énergétique avec RK4 pour les deux modèles
 */
int main_energie_analysis_rk4();

#endif
