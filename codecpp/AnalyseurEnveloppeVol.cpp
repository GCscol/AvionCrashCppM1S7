#include "AnalyseurEnveloppeVol.h"
#include "Avion.h"
#include "Constantes.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

AnalyseurEnveloppeVol::AnalyseurEnveloppeVol(Avion& av) : avion(av) {}

void AnalyseurEnveloppeVol::analyser_limites_vitesse() {
    using namespace Physique;
    
    cout << "Alt (m) | Vmax (m/s) | Mach_max | Vmin (m/s) | Mach_min | Vmin_th (m/s)" << endl;
    
    ofstream csv("vmax_analysis.csv");
    csv << "altitude,v_max_m_s,Mach_max,v_min,Mach_min,v_min_th\n";
    
    vector<double> altitudes = {5000, 6000, 7000, 8000, 9000, 10000, 
                                 11000, 12000, 13000, 14000, 15000};
    
    for (double alt : altitudes) {
        avion.z_ref() = alt;
        double rho = avion.get_env().calculer_rho(alt);
        double v_max = 500.0;  // Vitesse initiale maximale à tester
        double v_min = 10.0;
        
        double S = avion.get_aero().get_surface();
        double W = avion.get_masse() * g;
        double alpha_stall_rad = 15.0 * DEG_TO_RAD;
        double CL_max = 5.0 * (alpha_stall_rad - (-0.035)) + 0.44 * (-0.13);  // ||||||||||||| braquage de gouverne à -0,13 |||||||||||||||||||
        
        double v_min_theorique = std::sqrt(2.0 * W / (rho * S * CL_max));
        
        for (double v_test = v_max; v_test >= 150.0; v_test -= 0.1) {
            double delta_p = avion.trouver_delta_profondeur(v_test, 0); // calcul le plus long de la boucle
            double F_available = avion.calculer_poussee_max(v_test, rho, alt); // Poussée disponible à v_test
            
            double CL_needed = (2.0 * W) / (rho * v_test * v_test * S);   // CL nécessaire pour sustentation (L = W)
            if (CL_needed > CL_max) continue;  // Vérifier que CL_needed est réalisable (inférieur à CL_max)
            
            double alpha_needed = ((CL_needed - 0.44 * delta_p) / 5.0) + (-0.035);
            
            avion.get_aero().update_from_polar(alpha_needed, -0.13, 0, v_test);
            double D = avion.get_aero().calculer_trainee(v_test, rho);
            
            if (F_available <= D) v_max = v_test; // Vérifier l'équilibre F = D
            else if (F_available > D) v_min = v_test;
            
            if (v_test < 10.0) break;
            if (v_min > v_max) std::swap(v_min, v_max); // Si le coin supérieur du domaine de vol est atteint
        }
        
        double sound_speed = avion.get_env().calculer_vitesse_son(alt);
        double Mach_max = v_max / sound_speed;
        double Mach_min = v_min / sound_speed;
        
        cout << fixed << "\n";
        cout << alt << " | " << v_max << " | " << Mach_max 
             << " | " << v_min << " | " << Mach_min 
             << " | " << v_min_theorique << endl;
             
        csv << alt << "," << v_max << "," << Mach_max << "," 
            << v_min << "," << Mach_min << "," << v_min_theorique << endl;
    }
    
    csv.close();
    cout << "Résultats sauvegardés dans 'vmax_analysis.csv'" << endl;
}
