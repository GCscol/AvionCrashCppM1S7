#include "SystemeControle.h"
#include <algorithm>

SystemeControle::SystemeControle()
    : cmd_profondeur(0.0), cmd_thrust(0.667520), delta_p_max(0.15) {}

void SystemeControle::set_commande_profondeur(double val) {
    cmd_profondeur = std::clamp(val, -1.0, 1.0);   ///j'ai essayé d'intégrer ça mais pas sûr
}

void SystemeControle::set_commande_thrust(double val) {
    cmd_thrust = std::clamp(val, 0.0, 1.0);
}

double SystemeControle::get_cmd_profondeur() const {
    return cmd_profondeur;
}

double SystemeControle::get_cmd_thrust() const {
    return cmd_thrust;
}

double SystemeControle::get_delta_p_max() const {
    return delta_p_max;
}

void SystemeControle::reset() {
    cmd_profondeur = 0.0;
    cmd_thrust = 0.667520;
}
