#include "Environnement.h"
#include "Constantes.h"
#include <cmath>
#include <algorithm>

double Environnement::calculer_rho(double altitude) const {
    altitude = std::max(0.0, altitude);  /// Chelou si altitude négative, on devrait plutot faire une manière + propre et vraiment orientée gestion d'erreur rare plutot que ce truc qui applatit et cache
    double T = T_0 - L * altitude;
    T = std::max(T, T_min);   /// Pareil chelou, ça veut dire qu'on sort de la stratosphère, on devrait faire une gestion d'erreur rare + clean 
    
    return rho_0 * std::pow(T / T_0, Physique::g / (R * L) - 1);
}


/// Je l'ai mis car elle sortait parfois de manière récurrente mais pas encore convaincu de sa place ici
/// Je me suis juste dis si on changeait certaines variables avec un modèle + raffiner c''est plus clean de l'avoir ici
double Environnement::calculer_vitesse_son(double altitude) const {
    return 340.0 - 0.0065 * altitude;    /// + ça serait cool de la mettre en vrai constante physique
}
