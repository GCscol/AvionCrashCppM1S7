#include "ProprietesInertie.h"
#include "Constantes.h"

ProprietesInertie::ProprietesInertie(double m, double inertie_y)
    : masse(m), I_y(inertie_y) {}

double ProprietesInertie::get_masse() const {
    return masse;
}

double ProprietesInertie::get_I_pitch() const {
    return I_y;
}

double ProprietesInertie::calculer_poids() const {
    return masse * Physique::g;
}

void ProprietesInertie::set_masse(double m) {
    masse = m;
}
