#ifndef ANALYSEUR_ENERGIE_H
#define ANALYSEUR_ENERGIE_H

#include <string>
#include "Avion.h"

// Analyze system energy evolution during simulation
int analyser_energie_simulation(const std::string& fichier_csv_sortie,
                                double pas_temps,
                                double duree_sim,
                                double commande_profondeur,
                                double commande_thrust,
                                double cmd_start,
                                double cmd_end,
                                bool use_hysteresis);

// Analyze energy with RK4 integration (4th order Runge-Kutta)
int analyser_energie_simulation_rk4(const std::string& fichier_csv_sortie,
                                    double pas_temps,
                                    double duree_sim,
                                    double commande_profondeur,
                                    double commande_thrust,
                                    double cmd_start,
                                    double cmd_end,
                                    bool use_hysteresis);

int main_energie_analysis();

// Launch energy analysis with RK4 for both models
int main_energie_analysis_rk4();

#endif
