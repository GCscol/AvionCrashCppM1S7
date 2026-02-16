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

int main_energie_analysis();

#endif
