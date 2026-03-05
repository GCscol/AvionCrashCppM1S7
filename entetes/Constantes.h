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
enum class Rescue_Strategy { THRUST_FIRST, PROFILE_FIRST, SIMULTANEOUS };

struct Config {
    private :
        std::unordered_map<std::string, std::string> params;
        bool locked = false;

    public :
        Config() = default;

        void chargerDepuisFichier(const std::string& filename);
        void completer();
        void exporter(const std::string& filename) const;

        // Getters de base
        double getDouble(const std::string& key) const;
        bool   getBool  (const std::string& key) const;
        std::string getString(const std::string& key) const;
        void setString(const std::string& key, const std::string& value);
        // Getters pour Enum
        template<typename T>
        T getEnum(const std::unordered_map<std::string, T>& table, const std::string& key) const {
            auto it = table.find(params.at(key));
                if (it == table.end())
                    throw std::runtime_error("Valeur inconnue pour '" + key + "' : " + params.at(key));
                return it->second;
            }
        // Getters pour savoir qu'elle opération menée dans le main
        bool hasOperations(const std::string& op) const;
};

// Fonction de fabrique qui retourne un config const — plus modifiable ensuite
const Config chargerConfig(const std::string& filename);

// 
std::string check_output_file(const std::string& path);

// Tables de conversion — déclarées ici, définies dans le .cpp
extern const std::unordered_map<std::string, Methode_Integration> STR_TO_METHODE;
extern const std::unordered_map<std::string, Rescue_Strategy> STR_TO_RESCUE_STRATEGY;

extern Config config;   // extern = si on appelle constantes on a direct config qui est chargé et prêt à l'emploi

#endif  // CONSTANTES_H
