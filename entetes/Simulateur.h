#ifndef SIMULATEUR_H
#define SIMULATEUR_H

#include <string>
#include <limits>

class Avion;

class Simulateur {
private:
    Avion& avion;
    double dt;
    double temps_total;
    std::string fichier_sortie;
    // optional test commands
    double test_cmd_profondeur;
    double test_cmd_thrust;
    double test_cmd_start;
    double test_cmd_end;
    bool enable_rescue;  // Activation du système de sauvetage automatique
    double temps_debut_sauvetage;  // Temps où le sauvetage a démarré
    
public:
    Simulateur(Avion& av,
               double pas_temps = 0.01,
               double duree = 2000.0,
               const std::string& fichier = "simulation_full.csv",
               double cmd_profondeur = std::numeric_limits<double>::quiet_NaN(),
               double cmd_thrust = std::numeric_limits<double>::quiet_NaN(),
               double cmd_start = 100.0,
               double cmd_end = 500.0,
               bool enable_rescue_system = false);
    
    // Executes the simulation. Returns crash time (seconds) or NaN if no crash.
    double executer();
};

#endif // SIMULATEUR_H
