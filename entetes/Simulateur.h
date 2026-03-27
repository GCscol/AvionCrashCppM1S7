#ifndef SIMULATEUR_H
#define SIMULATEUR_H

#include <string>
#include <limits>
#include "OptiSauvetageGeneral.h"

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
    double seuil_altitude_critique;  // Seuil d'altitude pour activation sauvetage
    // Informations sur la récupération (remplies si un sauvetage a été détecté)
    double derniere_altitude_recuperation=0.0; // 0.0 = pas de crash pas de sauvetage, si positif sauvetage si neg crash
    double dernier_temps_recuperation = 0.0;    // 0.0 = non défini  Ou t crash si altitude negative
   
    const double temps_max_essai_sauvetage=60.0;
    
public:
    Simulateur(Avion& av,
               double pas_temps = 0.01,
               double duree = 2000.0,
               const std::string& fichier = "simulation_full.csv",
               double cmd_profondeur = std::numeric_limits<double>::quiet_NaN(),
               double cmd_thrust = std::numeric_limits<double>::quiet_NaN(),
               double cmd_start = 100.0,
               double cmd_end = 500.0,
               bool enable_rescue_system = false,
               double altitude_critique = 7500.0);


    double get_dernier_temps_recuperation() const;
    double get_derniere_altitude_recuperation() const;
    
    // Executes the simulation. Returns crash time (seconds) or NaN if no crash.
    double executer(OptiSauvetageGeneral::ParamsRescue* chromo=nullptr);
};

#endif // SIMULATEUR_H
