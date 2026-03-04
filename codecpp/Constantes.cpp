#include "Constantes.h"
#include <fstream>
#include <sstream>

Config config; 

// ----------------------------------------------------------------
// Tables de conversion string → enum, définies une seule fois
// ----------------------------------------------------------------
const std::unordered_map<std::string, Methode_Integration> STR_TO_METHODE = {
    {"EULER", Methode_Integration::EULER},
    {"RK4",   Methode_Integration::RK4},
};

const std::unordered_map<std::string, Autre> STR_TO_AUTRE = {
    {"Ex1",  Autre::Ex1},
    {"Ex2", Autre::Ex2},
};

// ----------------------------------------------------------------
// Chargement
// ----------------------------------------------------------------
void Config::chargerDepuisFichier(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Impossible d'ouvrir : " + filename);
    if (locked) throw std::runtime_error("Configuration verrouillée (normalement déjà chargée).");

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value))
            params[key] = value;
    }
}

// ----------------------------------------------------------------
// Complétion des valeurs manquantes
// ----------------------------------------------------------------
void Config::completer() {
    if (locked) throw std::runtime_error("Configuration verrouillée (normalement déjà chargée).");

    if (params.find("operations") == params.end() || params.at("operations").empty())   // Vérification s'il y a bien des opérations à réaliser, sinon on arrête directement
        throw std::runtime_error("Aucune opération à réaliser spécifiée dans la config : ajouter ou remplir la clé 'operations'");

    // On remplit les paramètres manquants non critiques avec des valeurs par défaut
    params.insert({"g",                   "9.81"});
    params.insert({"z_t",                 "2.0"});
    params.insert({"surface",                   "361.6"});
    params.insert({"corde",                 "6.6"});
    params.insert({"masse",                  "205000.0"});
    params.insert({"vx",               "240.0"});
    params.insert({"z",               "10670.0"});
    params.insert({"methode_integration", "RK4"});
    params.insert({"useHysteresis",         "false"});
    params.insert({"cmd_profondeur",                  "-0.8"});
    params.insert({"cmd_thrust",               "1.0"});
    params.insert({"cmd_start",               "60"});
    params.insert({"cmd_end", "600"});
    params.insert({"enable_rescue_system",         "false"});
    params.insert({"dt",                  "0.01"});
    params.insert({"duree",               "600.0"});
    params.insert({"Autre",         "Ex1"});

    // On vérouille la config pour éviter les modifications
    locked = true;
}

// ----------------------------------------------------------------
// Export
// ----------------------------------------------------------------
void Config::exporter(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Impossible d'écrire : " + filename);

    file << "# Config de simulation — générée automatiquement\n";
    for (const auto& pair : params)
        file << pair.first << "=" << pair.second << "\n";
}

/// 
///  Chargement config de manière const
/// 
const Config chargerConfig(const std::string& filename) {
    Config c;
    c.chargerDepuisFichier(filename);
    c.completer();
    return c;  // const dès la réception
}



double Config::getDouble(const std::string& key) const {
    return std::stod(params.at(key));

    //try {
    //    return std::stod(params.at(key));
    //} catch (const std::out_of_range&) {
    //    throw std::runtime_error("Clé '" + key + "' introuvable dans la config.");
    //} catch (const std::invalid_argument&) {
    //    throw std::runtime_error("Valeur invalide pour '" + key + "' : " + params.at(key));
    //}
    //}
}



bool Config::getBool(const std::string& key) const {
    return params.at(key) == "true";
}

bool Config::hasOperations(const std::string& op) const {
    const std::string ops = "," + params.at("operations") + ",";   // On ajoute une virgule au début et à la fin puis on cherche entre 2 virgules afin d'accélérer la recherche et éviter des problèmes si un nom est contenu dans l'autre : exemple Simu,Simu_batch
    return ops.find("," + op + ",") != std::string::npos;
}