#include "Integration.h"
#include "Avion.h"
#include "EtatCinematique.h"
#include "ForcesAerodynamiques.h"
#include "Constantes.h"
#include <cmath>


void Integration::Euler(Avion& avion, double dt) {

    ForcesAerodynamiques& forces = avion.get_forces();   // verifier au niveau rapidité ce qui est préférable
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
    
    avion.get_aero().update_from_polar(alpha, delta_p, avion.get_omega_pitch(), vitesse);
    avion.calculer_forces();
    forces.M_pitch = avion.get_aero().calculer_moment_pitch(vitesse, rho);  ///m'affiche comme si ça n'allait pas marcher alors que j'ai un non constant getter
    
        // Forces 
    /// potentiellement utiliser ForcesAerodynamiques& F = avion.get_forces();
    forces.Fx = forces.traction * (std::cos(yaw) * std::cos(pitch)) 
              - forces.trainee * (std::cos(yaw) * std::cos(gamma)) 
              - forces.portance * (std::sin(gamma) * std::cos(roll));
    
    forces.Fy = forces.traction * (std::sin(yaw) * std::cos(pitch)) 
              - forces.trainee * (std::sin(yaw) * std::cos(gamma)) 
              + forces.portance * (std::sin(gamma) * std::sin(roll));
    
    forces.Fz =forces.traction * std::sin(pitch) 
              - forces.trainee * std::sin(gamma) 
              + forces.portance * std::cos(gamma) 
              - forces.poids;
    
        // Intégration  
    double accel_x = forces.Fx / avion.get_masse();
    double accel_y = forces.Fy / avion.get_masse();
    double accel_z = forces.Fz / avion.get_masse();
    
    etat.vx += accel_x * dt;
    etat.vy += accel_y * dt;
    etat.vz += accel_z * dt;
    
    etat.x += avion.get_vitesse_x() * dt;
    etat.y += avion.get_vitesse_y() * dt;
    etat.z += avion.get_vitesse_z() * dt;
    
    double accel_pitch = avion.get_M_pitch() / avion.get_I_pitch();
    etat.omega_pitch += accel_pitch * dt;
    etat.pitch += avion.get_omega_pitch() * dt;
}


void Integration::RK4(Avion& avion, double dt) {

}