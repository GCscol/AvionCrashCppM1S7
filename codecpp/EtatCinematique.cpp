#include "EtatCinematique.h"
#include <cmath>

EtatCinematique::EtatCinematique()
    : x(0.0), y(0.0), z(0.0),
      vx(0.0), vy(0.0), vz(0.0),
      roll(0.0), pitch(0.0), yaw(0.0),
      omega_pitch(0.0) {}


double EtatCinematique::get_vitesse_norme() const {
    return std::sqrt(vx*vx + vy*vy + vz*vz);
}

double EtatCinematique::get_gamma() const {
    double vh = std::sqrt(vx*vx + vy*vy);
    return std::atan2(vz, vh);
}

double EtatCinematique::get_alpha() const {
    return pitch - get_gamma();
}
