#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <cmath>

namespace Physique {
    constexpr double RAD_TO_DEG = 180.0 / M_PI;
    constexpr double DEG_TO_RAD = M_PI / 180.0;
    constexpr double g = 9.81;
}

class Type_Integration {
    public:
    enum class Methode {
        EULER,
        RK4
    };
} ;

namespace Param_Simulation {
    extern Type_Integration::Methode methode_integration;   ///Seule manière d'avoir ce truc de merde qui marche, de toute facon ça sera change ( le pb etait sinon en def multiple)
}

#endif // CONSTANTES_H
