#ifndef ANALYSEUR_ENERGIE_H
#define ANALYSEUR_ENERGIE_H

#include <string>
#include "Avion.h"

// Structure encapsulant les paramètres de simulation énergétique
struct ParamsSimulationEnergie {
    std::string fichier_sortie = "output/energie_simulation.csv";
    double pas_temps = 0.01;
    double duree = 600.0;
    double cmd_profondeur = -0.8;
    double cmd_thrust = 1.0;
    double cmd_debut = 60.0;
    double cmd_fin = 600.0;
    bool use_hysteresis = false;
};

// Analyze system energy evolution during simulation
void analyser_energie_simulation(const ParamsSimulationEnergie& params);

// Analyze energy with RK4 integration (4th order Runge-Kutta)
void analyser_energie_simulation_rk4(const ParamsSimulationEnergie& params);

void main_energie_analysis();
void main_energie_analysis_rk4();

#endif
