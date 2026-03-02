#include "SystemeControle.h"
#include <algorithm>

SystemeControle::SystemeControle()
    : cmd_profondeur(0.0), cmd_thrust(0.667520), delta_p_max(0.4) {}

void SystemeControle::set_commande_profondeur(double val) {
    if (val < -1.0) cmd_profondeur = -1.0;
    else if (val > 1.0) cmd_profondeur = 1.0;
    else cmd_profondeur = val;
}

void SystemeControle::set_commande_thrust(double val) {
    if (val < 0.0) cmd_thrust = 0.0;
    else if (val > 1.0) cmd_thrust = 1.0;
    else cmd_thrust = val;
}

void SystemeControle::set_delta_p_max(double val) {
    if (val < 0.05) delta_p_max = 0.05;
    else if (val > 1.0) delta_p_max = 1.0;
    else delta_p_max = val;
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
