#include "SauvetageAvion.h"
#include <cmath>
#include <algorithm>
#include <limits>

constexpr double SauvetageAvion::SEUIL_DESCENTE_CRITIQUE;
constexpr double SauvetageAvion::SEUIL_ALTITUDE_CRITIQUE;
constexpr double SauvetageAvion::ASSIETTE_PULLUP_MAX;
constexpr double SauvetageAvion::CMD_PROFONDEUR_PULLUP;
constexpr double SauvetageAvion::CMD_POUSSEE_SAUVETAGE;
constexpr double SauvetageAvion::SEUIL_PITCH_TROP_NEGATIF;
constexpr double SauvetageAvion::CMD_POUSSEE_REDUITE;
constexpr double SauvetageAvion::TEMPS_MANOEUVRE_MIN;

SauvetageAvion::EtatSauvetage SauvetageAvion::evaluer_etat(
    const EtatCinematique& etat, double temps_courant, double derniere_manoeuvre) {
    
    EtatSauvetage evalue;
    evalue.taux_descente = etat.vz;
    evalue.altitude = etat.z;
    evalue.vitesse = etat.get_vitesse_norme();
    evalue.assiette = etat.pitch;
    evalue.temps_depuis_manoeuvre = temps_courant - derniere_manoeuvre;
    
    // Détecte descente critique : déclenche uniquement si l'avion est en descente
    // AND s'il est en altitude élevée (on cible la phase de chute après un pic)
    evalue.en_descente_critique = (etat.vz < SEUIL_DESCENTE_CRITIQUE) && (etat.z > SEUIL_ALTITUDE_CRITIQUE);
    
    // Détecte surcharge moteur (assiette négative extrême = perte de portance)
    evalue.moteur_en_surcharge = etat.pitch < -0.2;  // rad, pitch très négatif
    
    return evalue;
}

std::pair<double, double> SauvetageAvion::strategie_pullup_progressif(const EtatSauvetage& etat) {
    /**
     * Stratégie douce: augmente progressivement l'assiette (pitch positif)
     */
    double t = std::min(etat.temps_depuis_manoeuvre, TEMPS_MANOEUVRE_MIN);
    double fraction = t / TEMPS_MANOEUVRE_MIN;  // 0 -> 1
    
    double cmd_profondeur = (-CMD_PROFONDEUR_PULLUP) * fraction; // augmente progressivement pour créer un assiette positive
    double cmd_poussee = (etat.assiette < SEUIL_PITCH_TROP_NEGATIF) ? CMD_POUSSEE_REDUITE : CMD_POUSSEE_SAUVETAGE;
    
    return {cmd_profondeur, cmd_poussee};
}

std::pair<double, double> SauvetageAvion::strategie_poussee_max(const EtatSauvetage& etat) {
    /**
     * Stratégie agressive: maximum de poussée + assiette de pull-up IMMÉDIAT (quand l'altitude est très critique.)
     */
    double cmd_profondeur = -CMD_PROFONDEUR_PULLUP;  // Maximal immédiatement
    double cmd_poussee = (etat.assiette < SEUIL_PITCH_TROP_NEGATIF) ? CMD_POUSSEE_REDUITE : CMD_POUSSEE_SAUVETAGE;  // Pleine puissance sauf si nez trop bas
    
    return {cmd_profondeur, cmd_poussee};
}

std::pair<double, double> SauvetageAvion::strategie_manoeuvre_coordonnee(const EtatSauvetage& etat) {
    /**
     * Stratégie équilibrée: MAX poussée immédiate + assiette progressive.
     */
    double t = std::min(etat.temps_depuis_manoeuvre, TEMPS_MANOEUVRE_MIN);
    double fraction = t / TEMPS_MANOEUVRE_MIN;
    
    double cmd_poussee = (etat.assiette < SEUIL_PITCH_TROP_NEGATIF) ? CMD_POUSSEE_REDUITE : CMD_POUSSEE_SAUVETAGE; // réduire si nez trop bas
    double cmd_profondeur = (-CMD_PROFONDEUR_PULLUP) * fraction; // augmente progressivement pour contrôler la montée (-1.0 * fraction -> cabrage progressif)
    
    return {cmd_profondeur, cmd_poussee};
}

std::pair<double, double> SauvetageAvion::appliquer_sauvetage(const EtatSauvetage& etat, int strategie_id) {
    // Si pas en crise: retourne commandes neutres
    if (!etat.en_descente_critique) {
        return {0.0, 0.0};
    }
    
    // Auto-sélection de la stratégie si id = 0
    if (strategie_id == 0) {
        // Sélectionne selon la gravité de l'altitude
        if (etat.altitude < 300.0) {
            strategie_id = 2;  // Poussée max
        } else if (etat.altitude < 800.0) {
            strategie_id = 3;  // Manoeuvre coordonnée
        } else {
            strategie_id = 1;  // Pull-up progressif
        }
    }
    
    // Applique la stratégie choisie
    switch (strategie_id) {
        case 1:
            return strategie_pullup_progressif(etat);
        case 2:
            return strategie_poussee_max(etat);
        case 3:
            return strategie_manoeuvre_coordonnee(etat);
        default:
            return {0.0, 0.0};
    }
}

bool SauvetageAvion::verifier_succes_sauvetage(const EtatCinematique& etat_courant,
                                               const EtatCinematique& etat_initial_sauvetage,
                                               double temps_ecoule) {
    /**
     * Vérifie si la manoeuvre a réussi:
     * - Taux de descente ralentit (vz moins négatif)
     * - Critères moins stricts pour permettre succès
     */
    
    if (temps_ecoule < 1.5) {
        return false;  // Trop tôt pour juger
    }
    
    // Critère principal: la descente doit ralentir (vz moins négatif)
    // C'est le signe que le sauvetage fonctionne
    bool descente_ralentie = etat_courant.vz > (etat_initial_sauvetage.vz + 0.2);
    
    // Critère secondaire: altitude ne doit pas s'effondrer
    double delta_altitude = etat_courant.z - etat_initial_sauvetage.z;
    bool altitude_acceptable = delta_altitude > -200.0;  // Permet jusqu'à 200m de perte
    
    return descente_ralentie && altitude_acceptable;
}
