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
    //constexpr double dt= ; // à mettre en place
    Type_Integration::Methode methode_integration = Type_Integration::Methode::RK4;
}

#endif // CONSTANTES_H
