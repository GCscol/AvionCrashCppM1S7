#include "Avion.h"
#include "Simulateur.h"
#include "Constantes.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Physique;

int main() {
    cout << "========== DIAGNOSTIC TRIM ET FORCES INITIALES ==========" << endl;
    
    // Créer avion identique à la simulation
    Avion avion(361.6, 6.6, 200000, true);
    avion.initialiser();
    
    cout << "\n1. ETAT INITIAL (avant trim):" << endl;
    cout << "   Altitude: " << avion.get_altitude() << " m" << endl;
    cout << "   Vitesse vx: " << avion.get_vitesse_x() << " m/s" << endl;
    cout << "   Pitch: " << avion.get_pitch() * RAD_TO_DEG << " deg" << endl;
    cout << "   cmd_profondeur: " << avion.get_controle().get_cmd_profondeur() << endl;
    cout << "   cmd_thrust: " << avion.get_controle().get_cmd_thrust() << endl;
    
    // Calculer le trim comme le fait le Simulateur
    double speed = avion.get_vitesse_x();
    double alpha_trim = avion.trouver_alpha(speed);
    double delta_trim = avion.trouver_delta_profondeur(speed, avion.get_omega_pitch());
    
    avion.get_etat().pitch = alpha_trim;
    avion.get_aero().set_delta_profondeur(delta_trim);
    
    cout << "\n2. TRIM CALCULE:" << endl;
    cout << "   Alpha trim (pitch): " << alpha_trim * RAD_TO_DEG << " deg" << endl;
    cout << "   Delta profondeur trim: " << delta_trim << " rad" << endl;
    
    // Calculer les forces avec le trim
    double alpha = avion.get_etat().get_alpha();
    double rho = avion.get_env().calculer_rho(avion.get_altitude());
    double delta_p = avion.get_aero().get_delta_profondeur() 
                   + avion.get_cmd_profondeur() * avion.get_controle().get_delta_p_max();
    
    avion.get_aero().update_from_polar(alpha, delta_p, avion.get_omega_pitch(), speed);
    avion.calculer_forces();
    
    double M_aero = avion.get_aero().calculer_moment_pitch(speed, rho);
    const double z_t = 2.0;
    double M_thrust = z_t * avion.get_traction();
    
    cout << "\n3. FORCES AU TRIM (t=0, avec cmd_thrust=" << avion.get_controle().get_cmd_thrust() << "):" << endl;
    cout << "   Portance: " << avion.get_portance() << " N" << endl;
    cout << "   Poids: " << avion.get_masse() * g << " N" << endl;
    cout << "   Desequilibre vertical: " << (avion.get_portance() - avion.get_masse() * g) << " N" << endl;
    cout << endl;
    cout << "   Trainee: " << avion.get_trainee() << " N" << endl;
    cout << "   Traction: " << avion.get_traction() << " N" << endl;
    cout << "   Desequilibre horizontal: " << (avion.get_traction() - avion.get_trainee()) << " N" << endl;
    cout << endl;
    cout << "   Moment aero: " << M_aero << " N.m" << endl;
    cout << "   Moment thrust: " << M_thrust << " N.m" << endl;
    cout << "   Moment total: " << (M_aero + M_thrust) << " N.m" << endl;
    
    // Calculer l'accélération verticale attendue
    double gamma = avion.get_etat().get_gamma();
    double pitch = avion.get_pitch();
    double Fz = avion.get_traction() * sin(pitch) 
              - avion.get_trainee() * sin(gamma) 
              + avion.get_portance() * cos(gamma) 
              - avion.get_masse() * g;
    
    double accel_z = Fz / avion.get_masse();
    
    cout << "\n4. ANALYSE DYNAMIQUE:" << endl;
    cout << "   Force verticale nette (Fz): " << Fz << " N" << endl;
    cout << "   Acceleration verticale: " << accel_z << " m/s²" << endl;
    cout << "   Taux descente predit (apres 1s): " << accel_z << " m/s" << endl;
    
    if (accel_z < -0.1) {
        cout << "\n   ⚠️  L'avion va DESCENDRE des le debut !" << endl;
        cout << "   Raison: Desequilibre des forces verticales" << endl;
    } else if (accel_z > 0.1) {
        cout << "\n   ⚠️  L'avion va MONTER des le debut !" << endl;
    } else {
        cout << "\n   ✓ L'avion devrait rester stable (trim correct)" << endl;
    }
    
    cout << "\n========== ANALYSE ==========" << endl;
    cout << "Le trim est calcule en supposant traction = trainee." << endl;
    cout << "Mais cmd_thrust par defaut = " << avion.get_controle().get_cmd_thrust() << endl;
    cout << "Cela peut creer un desequilibre si cette poussee ne correspond pas" << endl;
    cout << "a la trainee calculee dans le trim." << endl;
    
    return 0;
}
