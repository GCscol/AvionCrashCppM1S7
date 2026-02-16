#include "Avion.h"
#include "Simulateur.h"
#include "Constantes.h"
#include <iostream>
#include <iomanip>
#include <utility>

using namespace std;
using namespace Physique;

int main() {
    cout << "========== DIAGNOSTIC TRIM COMPLET VIA SIMULATEUR ==========" << endl;
    
    // Créer avion identique à la simulation main
    Avion avion(361.6, 6.6, 200000, true);
    avion.initialiser();
    
    cout << "\n1. ETAT AVANT TRIM:" << endl;
    cout << "   cmd_thrust initial: " << avion.get_controle().get_cmd_thrust() << endl;
    
    // Appliquer le trim avec la nouvelle méthode complète
    double speed = avion.get_vitesse_x();
    std::pair<double, double> trim_result = avion.calculer_trim_complet(speed);
    double alpha_trim = trim_result.first;
    double delta_trim = trim_result.second;
    
    avion.get_etat().pitch = alpha_trim;
    avion.get_aero().set_delta_profondeur(delta_trim);
    
    // Calculer cmd_thrust pour équilibrer
    double rho = avion.get_env().calculer_rho(avion.get_altitude());
    avion.get_aero().update_from_polar(alpha_trim, delta_trim, 0.0, speed);
    double trainee_trim = avion.get_aero().calculer_trainee(speed, rho);
    double traction_max = avion.calculer_poussee_max(speed, rho, avion.get_altitude());
    double cmd_thrust_trim = trainee_trim / traction_max;
    avion.get_controle().set_commande_thrust(cmd_thrust_trim);
    
    cout << "\n2. TRIM APPLIQUE:" << endl;
    cout << "   Alpha: " << alpha_trim * RAD_TO_DEG << " deg" << endl;
    cout << "   Delta profondeur: " << delta_trim << " rad" << endl;
    cout << "   cmd_thrust: " << cmd_thrust_trim << endl;
    
    // Recalculer les forces avec le nouveau trim
    double alpha = avion.get_etat().get_alpha();
    double delta_p = avion.get_aero().get_delta_profondeur() 
                   + avion.get_cmd_profondeur() * avion.get_controle().get_delta_p_max();
    
    avion.get_aero().update_from_polar(alpha, delta_p, avion.get_omega_pitch(), speed);
    avion.calculer_forces();
    
    double M_aero = avion.get_aero().calculer_moment_pitch(speed, rho);
    const double z_t = 2.0;
    double M_thrust = z_t * avion.get_traction();
    
    cout << "\n3. FORCES APRÈS TRIM:" << endl;
    cout << "   Portance: " << avion.get_portance() << " N" << endl;
    cout << "   Poids: " << avion.get_masse() * g << " N" << endl;
    cout << "   Déséquilibre L-W: " << (avion.get_portance() - avion.get_masse() * g) << " N" << endl;
    cout << "   Ratio L/W: " << (avion.get_portance() / (avion.get_masse() * g)) << endl;
    cout << endl;
    cout << "   Traînée: " << avion.get_trainee() << " N" << endl;
    cout << "   Traction: " << avion.get_traction() << " N" << endl;
    cout << "   Déséquilibre T-D: " << (avion.get_traction() - avion.get_trainee()) << " N" << endl;
    cout << "   Ratio T/D: " << (avion.get_traction() / avion.get_trainee()) << endl;
    cout << endl;
    cout << "   Moment aéro: " << M_aero << " N.m" << endl;
    cout << "   Moment thrust: " << M_thrust << " N.m" << endl;
    cout << "   Moment TOTAL: " << (M_aero + M_thrust) << " N.m" << endl;
    
    // Simuler quelques pas pour voir l'évolution
    cout << "\n4. SIMULATION DES PREMIERS PAS DE TEMPS:" << endl;
    cout << "   t(s)    z(m)       vz(m/s)    pitch(deg)  L/W" << endl;
    
    for (int i = 0; i < 1000; ++i) {
        double t = i * 0.1;
        avion.mettre_a_jour_etat(0.1);
        double LW_ratio = avion.get_portance() / (avion.get_masse() * g);
        cout << "   " << fixed << setprecision(2) << t 
             << "     " << setprecision(1) << avion.get_altitude()
             << "     " << setprecision(3) << setw(8) << avion.get_vitesse_z()
             << "     " << setprecision(2) << setw(8) << (avion.get_pitch() * RAD_TO_DEG)
             << "   " << setprecision(4) << LW_ratio << endl;
    }
    
    return 0;
}
