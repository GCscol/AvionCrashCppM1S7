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
    
    cout << "Alt (m) | Vmax (m/s) | Mach_max | Vmin (m/s) | Mach_min" << endl;
    
    ofstream csv("vmax_analysis.csv");
    csv << "altitude,v_max_m_s,Mach_max,v_min,Mach_min\n";
    
    vector<double> altitudes = {5000, 6000, 7000, 8000, 9000, 10000, 
                                 11000, 12000, 13000, 14000, 15000};
    
    for (double alt : altitudes) {
        avion.z_ref() = alt;
        double rho = avion.get_env().calculer_rho(alt);
        double v_max = 500.0;  // Vitesse initiale maximale à tester
        double v_min;
        
        double S = avion.get_aero().get_surface();
        double W = avion.get_masse() * g;
        double alpha_stall_rad = 15.0 * DEG_TO_RAD;
        double CL_max = 5.0 * (alpha_stall_rad - (-0.035)) + 0.44 * (-0.13);  // ||||||||||||| braquage de gouverne à -0,13 |||||||||||||||||||
        
        v_min = std::sqrt(2.0 * W / (rho * S * CL_max));

        const double v_step = 0.1;
        const double v_low = v_min;
        const double v_high = 500.0;

        v_max = v_high;
        // Diagnostic : écrire un fichier détaillé pour alt >= 12000
        ofstream diag;
        bool write_diag = (alt >= 12000.0);
        if (write_diag) {
            diag.open("v_diagnostics.csv", std::ios::app);
            if (diag.tellp() == 0) {
                diag << "altitude,v_test,rho,CL_needed,delta_p,CL_after_trim,alpha_needed,F_available,D\n";
            }
        }

        for (double v_test = v_high; v_test >= v_low; v_test -= v_step) {
            double CL_needed = (2.0 * W) / (rho * v_test * v_test * S);
            if (CL_needed > CL_max) continue;

            double delta_p = avion.trouver_delta_profondeur(v_test, 0);
            double F_available = avion.calculer_poussee_max(v_test, rho, alt);

            double alpha_needed = ((CL_needed - 0.44 * delta_p) / 5.0) + (-0.035);
            avion.get_aero().update_from_polar(alpha_needed, -0.13, 0, v_test);
            double D = avion.get_aero().calculer_trainee(v_test, rho);

            if (write_diag) {
                double CL_after = avion.get_aero().C_L;
                diag << alt << "," << v_test << "," << rho << "," << CL_needed << "," << delta_p << "," << CL_after << "," << alpha_needed << "," << F_available << "," << D << "\n";
            }

            if (F_available > D) {
                v_max = v_test;
                break;
            }
        }
        

        if (v_max >= v_high - 1e-6) {
            v_max = v_min; // signaler incapacité à atteindre des vitesses > v_min
        }
        
        double sound_speed = avion.get_env().calculer_vitesse_son(alt);
        double Mach_max = v_max / sound_speed;
        double Mach_min = v_min / sound_speed;
        
        cout << fixed << "\n";
        cout << alt << " | " << v_max << " | " << Mach_max 
             << " | " << v_min << " | " << Mach_min << endl;
             
        csv << alt << "," << v_max << "," << Mach_max << "," 
            << v_min << "," << Mach_min << endl;
    }
    
    csv.close();
    cout << "Résultats sauvegardés dans 'vmax_analysis.csv'" << endl;
}