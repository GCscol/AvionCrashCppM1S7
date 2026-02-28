#ifndef TEST_INITIALIZATION_RANGE_H
#define TEST_INITIALIZATION_RANGE_H

#include <string>

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
 * @return 0 si succès, 1 si erreur
 */
int test_initialization_range(
    double vx_min, double vx_max, double vx_step,
    double z_min, double z_max, double z_step,
    bool useHysteresis = false,
    double sim_duration = 600.0,
    const std::string& output_file = "output/init_range_test.csv"
);

#endif // TEST_INITIALIZATION_RANGE_H
