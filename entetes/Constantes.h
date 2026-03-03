#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <string>
#include <unordered_map>  // Pour avoir table de conversion clé la valeur, depuis le fichier txt
#include <stdexcept>    // Pour avoir les exceptions standards
#include <fstream>     // Pour lire et écrire des fichiers
#include <sstream>     // Pour parser les lignes du fichier


namespace Math {
    constexpr double PI= 3.14159265358979323846;
    constexpr double RAD_TO_DEG = 180.0 / PI;
    constexpr double DEG_TO_RAD = PI / 180.0;
}

enum class Methode_Integration { EULER, RK4 };
enum class Autre          { Ex1, Ex2 };

struct Config {
    std::unordered_map<std::string, std::string> params;

    Config() = default;

    void chargerDepuisFichier(const std::string& filename);
    void completer();
    void exporter(const std::string& filename) const;

    // Getters de base
   double getDouble(const std::string& key) const;
    bool   getBool  (const std::string& key) const;
    // Getters pour Enum
    template<typename T>
    T getEnum(const std::unordered_map<std::string, T>& table, const std::string& key) const {
    auto it = table.find(params.at(key));
    if (it == table.end())
        throw std::runtime_error("Valeur inconnue pour '" + key + "' : " + params.at(key));
    return it->second;
    }
};

// Fonction de fabrique qui retourne un config const — plus modifiable ensuite
const Config chargerConfig(const std::string& filename);

// Tables de conversion — déclarées ici, définies dans le .cpp
extern const std::unordered_map<std::string, Methode_Integration> STR_TO_METHODE;
extern const std::unordered_map<std::string, Autre>         STR_TO_AUTRE;

extern Config config;

#endif  // CONSTANTES_H
