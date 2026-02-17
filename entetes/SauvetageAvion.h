#ifndef SAUVETAGE_AVION_H
#define SAUVETAGE_AVION_H

#include "EtatCinematique.h"
#include <utility>

// Rescue system: reduces commands progressively to recover before crash
class SauvetageAvion {
public:
    struct EtatSauvetage {
        bool en_descente_critique = false;
        double taux_descente = 0.0;         // Vertical rate (m/s)
        double altitude = 0.0;              // Current altitude (m)
        double vitesse = 0.0;               // Speed (m/s)
        double assiette = 0.0;              // Pitch (rad)
        double alpha = 0.0;                 // Angle of attack (rad)
        double temps_depuis_manoeuvre = 0.0;// Time since rescue start (s)
        
        double cmd_profondeur_max = 0.0;
        double cmd_thrust_max = 0.0;
    };

    static EtatSauvetage evaluer_etat(const EtatCinematique& etat, double temps_courant, 
                                      double cmd_prof_max, double cmd_thrust_max,
                                      double derniere_manoeuvre = -1e6);

    // Progressive reduction scenario: reduces commands gradually
    static std::pair<double, double> scenario_progressif(const EtatSauvetage& etat);

    // Apply rescue scenario to get normalized commands
    static std::pair<double, double> appliquer_sauvetage(const EtatSauvetage& etat);

    // Check if rescue was successful: vz > 0 for 2s, alpha < 14°, speed in [120, 350] m/s
    static bool verifier_succes_sauvetage(const EtatCinematique& etat_courant,
                                          const EtatCinematique& etat_initial_sauvetage,
                                          double temps_ecoule,
                                          double temps_vz_positif);

private:
    // Rescue thresholds and phase durations
    static constexpr double SEUIL_DESCENTE_CRITIQUE = -30.0;
    static constexpr double SEUIL_PITCH_CRITIQUE = -0.8;
    
    static constexpr double PHASE_REDUCTION_THRUST = 0.6;
    static constexpr double PHASE_REDUCTION_PROF = 0.6;
    static constexpr double PHASE_CONTROL = 3.0;

    static constexpr double THRUST_REDUCED_FACTOR = 0.2;
    static constexpr double PROF_REDUCED_FACTOR = -0.2;
};

#endif // SAUVETAGE_AVION_H
