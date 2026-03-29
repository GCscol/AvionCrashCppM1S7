#ifndef SAUVETAGE_AVION_H
#define SAUVETAGE_AVION_H

#include "EtatCinematique.h"
#include "OptiSauvetageGeneral.h"
#include <utility>


// Rescue system: reduces commands progressively to recover before crash
class SauvetageAvion {
public:
    struct Parametres {
        double phase_reduction_thrust = 5;
        double phase_reduction_prof = 5;
        double phase_control = 3.0;
        double thrust_reduced_factor = 0.2;
        double prof_reduced_factor = -0.2;
        double stabilization_thrust_factor = 0.6; // Thrust target ratio at end of rescue
    };

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
                                      double derniere_manoeuvre = -1e6,
                                      double seuil_altitude_critique = 7500.0);

    static void set_parametres(const Parametres& params);
    static Parametres get_parametres();
    static void reset_parametres_defaut();

    static void set_seuils_critiques(double seuil_descente, double seuil_pitch);
    static std::pair<double, double> get_seuils_critiques();
    static void reset_seuils_critiques_defaut();

    // Progressive reduction scenario: reduces commands gradually
    static std::pair<double, double> scenario_progressif(const EtatSauvetage& etat, OptiSauvetageGeneral::ParamsRescue* chromo= nullptr);

    // Apply rescue scenario to get normalized commands
    static std::pair<double, double> appliquer_sauvetage(const EtatSauvetage& etat, OptiSauvetageGeneral::ParamsRescue* chromo= nullptr);

    // Check if rescue was successful: vz > 0 for 2s, alpha < 14°, speed in [120, 350] m/s
    static bool verifier_succes_sauvetage(const EtatCinematique& etat_courant,
                                          const EtatCinematique& etat_initial_sauvetage,
                                          double temps_ecoule,
                                          double temps_vz_positif);

private:
    static constexpr double SEUIL_DESCENTE_CRITIQUE_DEFAUT = -30.0;
    static constexpr double SEUIL_PITCH_CRITIQUE_DEFAUT = -0.8;

    static Parametres params_actifs;
    static double seuil_descente_critique_actif;
    static double seuil_pitch_critique_actif;
};

#endif // SAUVETAGE_AVION_H
