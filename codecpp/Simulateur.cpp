#include "Simulateur.h"
#include "Avion.h"
#include "Constantes.h"
#include <iostream>
#include <fstream>

using namespace std;

// a verifier

Simulateur::Simulateur(Avion& av, double pas_temps, double duree,
                       const string& fichier)
    : avion(av), dt(pas_temps), temps_total(duree), fichier_sortie(fichier) {}

void Simulateur::executer() {
    using namespace Physique;
    
    // setlocale(LC_ALL, ".utf8");
    
    const int steps = static_cast<int>(temps_total / dt);
    
    std::ofstream csv(fichier_sortie);
    if (!csv.is_open()) {
        std::cerr << "Impossible d'ouvrir " << fichier_sortie << " en écriture\n";
        return;
    }
    
        // Initialisation du trim
    double speed = avion.get_vitesse_x();
    double alpha_trim = avion.trouver_alpha(speed);
    double delta_trim = avion.trouver_delta_profondeur(speed, avion.get_omega_pitch());
    
    avion.get_etat().pitch = alpha_trim;
    avion.get_aero().set_delta_profondeur(delta_trim);
    
    
    csv << "time,"
        << "x,y,z,vx,vy,vz,"
        << "roll,pitch,yaw,"
        << "M_pitch,I_pitch,omega_pitch,"
        << "Fx,Fy,Fz,portance,trainee,traction,"
        << "Cl,Cd,Cm,"
        << "speed,AoA_deg,cmd_profondeur,alpha,delta_profondeur,n_factor\n";
    
        // Boucle de simulation
    for (int i = 0; i < steps; ++i) {
        double t = (i + 1) * dt;
        avion.mettre_a_jour_etat(dt);
        
        double speed = avion.get_etat().get_vitesse_norme();
        double alpha = avion.get_etat().get_alpha();
        double AoA_deg = alpha * RAD_TO_DEG;
        
        // Commandes pilote 
        // if (t >= 100 && t < 500) {
        //     avion.get_controle().set_commande_profondeur(-0.55);
        //     avion.get_controle().set_commande_thrust(0.9);
        // }
        
        double n_factor = avion.get_portance() / (avion.get_masse() * g);
        
        csv << t << ',' 
            << avion.get_x() << ',' << avion.get_y() << ',' << avion.get_altitude() << ','
            << avion.get_vitesse_x() << ',' << avion.get_vitesse_y() << ',' << avion.get_vitesse_z() << ','
            << avion.get_roll() << ',' << avion.get_pitch() << ',' << avion.get_yaw() << ','
            << avion.get_M_pitch() << ',' << avion.get_I_pitch() << ',' << avion.get_omega_pitch() << ','
            << avion.get_Fx() << ',' << avion.get_Fy() << ',' << avion.get_Fz() << ','
            << avion.get_portance() << ',' << avion.get_trainee() << ',' << avion.get_traction() << ','
            << avion.get_aero().C_L << ',' << avion.get_aero().C_D << ',' << avion.get_aero().C_m << ','
            << speed << ',' << AoA_deg << ',' << avion.get_cmd_profondeur() << ','
            << alpha << ',' << avion.get_aero().get_delta_profondeur() << ',' << n_factor << '\n';
        
        if (avion.get_altitude() <= 0) {
            cout << "Crash !" << endl;
            break;
        }
    }
    
    csv.close();
    std::cout << "Simulation enregistrée dans : " << fichier_sortie << std::endl;
}
