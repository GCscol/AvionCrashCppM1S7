#ifndef SAUVETAGE_AVION_H
#define SAUVETAGE_AVION_H

#include "EtatCinematique.h"
#include <utility>

/**
 * Système de sauvetage pour tenter la récupération avant un crash.
 * 
 * Les commandes maximales (cmd_profondeur_max, cmd_thrust_max) sont des LIMITES.
 * En situation critique, le système réduit d'abord ces commandes (dans un ordre défini),
 * puis les fait varier pour regagner de l'altitude en douceur.
 * 
 * Un scénario est disponible:
 * PROGRESSIF: Réduction progressive → Augmentation progressive (douce)
 */
class SauvetageAvion {
public:
    /**
     * Structure pour retenir l'état et les métriques de sauvetage
     */
    struct EtatSauvetage {
        bool en_descente_critique = false;  // vz < seuil ET altitude correcte
        double taux_descente = 0.0;         // vz (m/s, négatif = descente)
        double altitude = 0.0;              // z (m)
        double vitesse = 0.0;               // norme
        double assiette = 0.0;              // pitch (rad)
        double alpha = 0.0;                 // angle d'attaque (rad)
        double temps_depuis_manoeuvre = 0.0;// temps écoulé depuis début sauvetage
        
        // Commandes maximales (reçues en paramètre)
        double cmd_profondeur_max = 0.0;
        double cmd_thrust_max = 0.0;
    };

    /**
     * Calcule l'état critique de l'avion
     */
    static EtatSauvetage evaluer_etat(const EtatCinematique& etat, double temps_courant, 
                                      double cmd_prof_max, double cmd_thrust_max,
                                      double derniere_manoeuvre = -1e6);

    /**
     * Scénario: "PROGRESSIF" (douce)
     * Réduit les commandes progressivement puis les augmente doucement
     * Préférable pour éviter les surcharges aérodynamiques
     * 
     * @param etat État courant incluant temps_depuis_manoeuvre
     * @return pair (cmd_profondeur, cmd_thrust) normalisées
     */
    static std::pair<double, double> scenario_progressif(const EtatSauvetage& etat);

    /**
     * Applique le scénario de sauvetage
     * 
     * @param etat État courant de l'avion
     * @return pair (cmd_profondeur, cmd_thrust) normalisées
     */
    static std::pair<double, double> appliquer_sauvetage(const EtatSauvetage& etat);

    /**
     * Vérifie si la manoeuvre de sauvetage est réussie selon des criteres physiques.
     */
    static bool verifier_succes_sauvetage(const EtatCinematique& etat_courant,
                                          const EtatCinematique& etat_initial_sauvetage,
                                          double temps_ecoule,
                                          double temps_vz_positif);

private:
    // Constantes de sauvetage - Trigger late, near ground when alternative is crash
    static constexpr double SEUIL_DESCENTE_CRITIQUE = -30.0;     // m/s (strong descent rate)
    static constexpr double SEUIL_ALTITUDE_CRITIQUE = 14000;    // m (trigger low to avoid interference)
    static constexpr double SEUIL_PITCH_CRITIQUE = -0.8;         // rad (~-46 deg, very steep)
    
    // Phases de récupération (tunable for testing different strategies)
    static constexpr double PHASE_REDUCTION_THRUST = 0.6;   // Reduce thrust duration (s)
    static constexpr double PHASE_REDUCTION_PROF = 0.6;     // Reduce profondeur duration (s)
    static constexpr double PHASE_CONTROL = 3.0;            // Stabilization phase (s) - OPTIMAL

    // Cibles de commandes pendant le sauvetage
    static constexpr double THRUST_REDUCED_FACTOR = 0.2;    // Reduced thrust level (less reduction = better)
    static constexpr double PROF_REDUCED_FACTOR = -0.2;     // Nose-up command target
};

#endif // SAUVETAGE_AVION_H
