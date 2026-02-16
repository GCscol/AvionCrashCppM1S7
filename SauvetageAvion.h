#ifndef SAUVETAGE_AVION_H
#define SAUVETAGE_AVION_H

#include "EtatCinematique.h"
#include <utility>

/**
 * Stratégies de sauvetage pour tenter de récupérer l'avion avant un crash.
 * Le but est de retrouver de la portance et reprendre de l'altitude progressivement.
 */
class SauvetageAvion {
public:
    /**
     * Structure pour retenir l'état et les métriques de sauvetage
     */
    struct EtatSauvetage {
        bool en_descente_critique = false;  // vz < seuil
        bool moteur_en_surcharge = false;   // poussée > seuil
        double taux_descente = 0.0;         // vz (m/s, négatif = descente)
        double altitude = 0.0;              // z (m)
        double vitesse = 0.0;               // norme
        double assiette = 0.0;              // pitch (rad)
        double temps_depuis_manoeuvre = 0.0; // temps écoulé depuis début sauvetage
    };

    /**
     * Calcule l'état critique de l'avion
     */
    static EtatSauvetage evaluer_etat(const EtatCinematique& etat, double temps_courant, double derniere_manoeuvre = -1e6);

    /**
     * Stratégie 1: "Pull-up progressif" 
     * Augmente graduellement l'assiette pour retrouver de la portance
     */
    static std::pair<double, double> strategie_pullup_progressif(const EtatSauvetage& etat);

    /**
     * Stratégie 2: "Poussée maximale + profondeur"
     * Augmente moteur au maximum et applique une assiette de récupération
     */
    static std::pair<double, double> strategie_poussee_max(const EtatSauvetage& etat);

    /**
     * Stratégie 3: "Manoeuvre coordonnée"
     * Combine augmentation progressive de poussée et variation douce de pitch
     */
    static std::pair<double, double> strategie_manoeuvre_coordonnee(const EtatSauvetage& etat);

    /**
     * Sélectionne et applique la stratégie la plus appropriée selon l'état
     * 
     * @param etat État courant de l'avion
     * @param strategie_id 1-3 pour choisir la stratégie, 0 pour auto-sélection
     * @return pair (cmd_profondeur, cmd_poussee) normalisées [-1, 1]
     */
    static std::pair<double, double> appliquer_sauvetage(const EtatSauvetage& etat, int strategie_id = 0);

    /**
     * Vérifie si la manoeuvre de sauvetage est réussie (altitude stable ou en hausse)
     */
    static bool verifier_succes_sauvetage(const EtatCinematique& etat_courant, 
                                          const EtatCinematique& etat_initial_sauvetage,
                                          double temps_ecoule);

private:
    // Constantes de sauvetage
    static constexpr double SEUIL_DESCENTE_CRITIQUE = -0.5;   // m/s (détection plus graduelle de la descente)
    static constexpr double SEUIL_ALTITUDE_CRITIQUE = 11500.0; // m (intervenir dès début de descente après pic de 14500m)
    // Si l'assiette est fortement négative (nez très bas), réduire la poussée
    static constexpr double SEUIL_PITCH_TROP_NEGATIF = -0.4; // rad (~-23°) — essayer intervention plus tard
    static constexpr double CMD_POUSSEE_REDUITE = 0.6;      // poussée réduite (modérée) quand nez est trop bas
    static constexpr double ASSIETTE_PULLUP_MAX = 0.3;       // rad (~17°)
    static constexpr double CMD_PROFONDEUR_PULLUP = -1.0;    // [-1, 1] (Cabrage pour augmenter portance)
    static constexpr double CMD_POUSSEE_SAUVETAGE = 1.0;     // [-1, 1] (MAXIMAL)
    static constexpr double TEMPS_MANOEUVRE_MIN = 1.0;       // secondes (RAPIDE)
};

#endif // SAUVETAGE_AVION_H
