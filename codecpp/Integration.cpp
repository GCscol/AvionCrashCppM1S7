#include "Integration.h"
#include "Avion.h"
#include "EtatCinematique.h"
#include "ForcesAerodynamiques.h"
#include "Constantes.h"
#include "ModeleHysteresis.h"
#include <cmath>


Integration::Derivees Integration::calculer_derivees(Avion& avion) {

    ForcesAerodynamiques& forces = avion.get_forces();
    EtatCinematique& etat = avion.get_etat();
    
    double vitesse = etat.get_vitesse_norme();
    double gamma = etat.get_gamma();
    double alpha = etat.get_alpha();
    double rho = avion.get_env().calculer_rho(avion.get_altitude());

    double yaw = avion.get_yaw();
    double pitch = avion.get_pitch();
    double roll = avion.get_roll();
    
    double delta_p = avion.get_aero().get_delta_profondeur() 
                   + avion.get_cmd_profondeur() * avion.get_controle().get_delta_p_max();
    
    // Update hysteresis model if present
    ModeleHysteresis* mh = dynamic_cast<ModeleHysteresis*>(&avion.get_aero());
    if (mh) {
        mh->update_from_polar(alpha, delta_p, avion.get_omega_pitch(), vitesse, 0.01);
    } else {
        avion.get_aero().update_from_polar(alpha, delta_p, avion.get_omega_pitch(), vitesse);
    }
    avion.calculer_forces();
    
    // Aerodynamic and thrust moments
    double M_aero = avion.get_aero().calculer_moment_pitch(vitesse, rho);
    const double z_t = 2;  // Lever arm (m)
    double M_thrust = z_t * forces.traction;
    forces.M_thrust = M_thrust;
    forces.M_pitch = M_aero + M_thrust;
    
    // Force components in body frame
    forces.Fx = forces.traction * (std::cos(yaw) * std::cos(pitch)) 
              - forces.trainee * (std::cos(yaw) * std::cos(gamma)) 
              - forces.portance * (std::sin(gamma) * std::cos(roll));
    
    forces.Fy = forces.traction * (std::sin(yaw) * std::cos(pitch)) 
              - forces.trainee * (std::sin(yaw) * std::cos(gamma)) 
              + forces.portance * (std::sin(gamma) * std::sin(roll));
    
    forces.Fz = forces.traction * std::sin(pitch) 
              - forces.trainee * std::sin(gamma) 
              + forces.portance * std::cos(gamma) 
              - forces.poids;

    // Calcul des dérivées
    Derivees d;
    d.dx = etat.vx;
    d.dy = etat.vy;
    d.dz = etat.vz;
    d.dvx = forces.Fx / avion.get_masse();
    d.dvy = forces.Fy / avion.get_masse();
    d.dvz = forces.Fz / avion.get_masse();
    d.dpitch = avion.get_omega_pitch();
    d.domega_pitch = avion.get_M_pitch() / avion.get_I_pitch();
    
    return d;
}

void Integration::Euler(Avion& avion, double dt) {

    EtatCinematique& etat = avion.get_etat();

    Derivees d = calculer_derivees(avion);
    
    // Attention ordre assignation etat_x et etat_vx (de même pour y et z et pitch)
    etat.x += d.dx * dt;   
    etat.y += d.dy * dt;
    etat.z += d.dz * dt;

    etat.vx += d.dvx * dt;   
    etat.vy += d.dvy * dt;   
    etat.vz += d.dvz * dt;   
    
    etat.pitch += d.dpitch * dt;
    etat.omega_pitch += d.domega_pitch * dt;
}


void Integration::RK4(Avion& avion, double dt) {
    
    EtatCinematique& etat = avion.get_etat();
    
    // Sauvegarde de l'état initial
    double x0 = etat.x, y0 = etat.y, z0 = etat.z;
    double vx0 = etat.vx, vy0 = etat.vy, vz0 = etat.vz;
    double pitch0 = etat.pitch, omega_pitch0 = etat.omega_pitch;

    // Liste des différentes k qui se baseront sur des temps et positions successives (incréments)
    Derivees k1 = calculer_derivees(avion);

    etat.x = x0 + 0.5 * dt * k1.dx;
    etat.y = y0 + 0.5 * dt * k1.dy;
    etat.z = z0 + 0.5 * dt * k1.dz;
    etat.vx = vx0 + 0.5 * dt * k1.dvx;
    etat.vy = vy0 + 0.5 * dt * k1.dvy;
    etat.vz = vz0 + 0.5 * dt * k1.dvz;
    etat.pitch = pitch0 + 0.5 * dt * k1.dpitch;
    etat.omega_pitch = omega_pitch0 + 0.5 * dt * k1.domega_pitch;
    Derivees k2 = calculer_derivees(avion);

    etat.x = x0 + 0.5 * dt * k2.dx;
    etat.y = y0 + 0.5 * dt * k2.dy;
    etat.z = z0 + 0.5 * dt * k2.dz;
    etat.vx = vx0 + 0.5 * dt * k2.dvx;
    etat.vy = vy0 + 0.5 * dt * k2.dvy;
    etat.vz = vz0 + 0.5 * dt * k2.dvz;
    etat.pitch = pitch0 + 0.5 * dt * k2.dpitch;
    etat.omega_pitch = omega_pitch0 + 0.5 * dt * k2.domega_pitch;
    Derivees k3 = calculer_derivees(avion);

    etat.x = x0 + dt * k3.dx;
    etat.y = y0 + dt * k3.dy;
    etat.z = z0 + dt * k3.dz;
    etat.vx = vx0 + dt * k3.dvx;
    etat.vy = vy0 + dt * k3.dvy;
    etat.vz = vz0 + dt * k3.dvz;
    etat.pitch = pitch0 + dt * k3.dpitch;
    etat.omega_pitch = omega_pitch0 + dt * k3.domega_pitch;  
    Derivees k4 = calculer_derivees(avion);

    // Combinaison finale
    etat.x = x0 + (k1.dx + 2*k2.dx + 2*k3.dx + k4.dx) * dt / 6.0;
    etat.y = y0 + (k1.dy + 2*k2.dy + 2*k3.dy + k4.dy) * dt / 6.0;
    etat.z = z0 + (k1.dz + 2*k2.dz + 2*k3.dz + k4.dz) * dt / 6.0;
    etat.vx = vx0 + (k1.dvx + 2*k2.dvx + 2*k3.dvx + k4.dvx) * dt / 6.0;
    etat.vy = vy0 + (k1.dvy + 2*k2.dvy + 2*k3.dvy + k4.dvy) * dt / 6.0;
    etat.vz = vz0 + (k1.dvz + 2*k2.dvz + 2*k3.dvz + k4.dvz) * dt / 6.0;
    etat.pitch = pitch0 + (k1.dpitch + 2*k2.dpitch + 2*k3.dpitch + k4.dpitch) * dt / 6.0;
    etat.omega_pitch = omega_pitch0 + (k1.domega_pitch + 2*k2.domega_pitch + 2*k3.domega_pitch + k4.domega_pitch) * dt / 6.0;
}