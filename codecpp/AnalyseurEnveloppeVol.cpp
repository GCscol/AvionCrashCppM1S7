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

/**
 * Check longitudinal dynamic stability using simplified eigenvalue analysis
 * Returns: true if stable, false if unstable
 */
bool AnalyseurEnveloppeVol::check_stability(double V, double alpha, double mach, 
                                             double rho, double delta_p) {
    using namespace Physique;
    
    const double m = avion.get_masse();
    const double S = avion.get_aero().get_surface();
    const double c = avion.get_aero().get_corde();
    const double I_y = avion.get_I_pitch();
    
    // Update aerodynamics with Mach effects
    avion.get_aero().update_from_polar(alpha, delta_p, 0.0, V, mach);
    
    // Aerodynamic derivatives (stability coefficients)
    const double C_L_alpha = 5.0;  // Lift curve slope (rad^-1)
    const double C_D_0 = 0.0175;
    const double K_induced = 0.055;
    
    // C_m_alpha depends on Mach (compressibility effect on aerodynamic center)
    const double x_f0 = 0.20;
    double x_f_compressibility = 0.0;
    if (mach > 0.3) {
        double k_mach = 0.80;
        x_f_compressibility = k_mach * (mach - 0.3) * (mach - 0.3);
        if (mach > 0.75) {
            x_f_compressibility += 0.5 * (mach - 0.75);
        }
    }
    double x_f = x_f0 - x_f_compressibility;  // Static margin DECREASES with Mach
    double C_m_alpha = -5.0 * x_f;  // Moment curve slope (NEGATIVE for stability when x_f > 0)
    
    // Pitch damping coefficient
    const double C_m_q = -12.0;
    
    // Dimensional stability derivatives
    double q_dyn = 0.5 * rho * V * V;
    double Z_alpha = -q_dyn * S * C_L_alpha;  // Negative (lift opposes alpha increase)
    double M_alpha = q_dyn * S * c * C_m_alpha;  // Should be negative for stability
    double M_q = 0.25 * rho * V * S * c * c * C_m_q;  // Pitch damping
    
    // Simplified short-period stability matrix (2x2)
    // States: [w, q] (vertical velocity perturbation, pitch rate)
    // A_sp = [Z_w/m,  V0  ]
    //        [M_w/Iy, M_q/I_y]
    
    double a11 = Z_alpha / (m * V);  // Z_w/m
    double a12 = V;                  // Coupling term
    double a21 = M_alpha / (I_y * V);  // M_w/Iy  
    double a22 = M_q / I_y;          // M_q/Iy (damping)
    
    // Stability criteria for 2x2 matrix:
    // 1. Trace < 0 (sum of eigenvalues negative)
    // 2. Determinant > 0 (product of eigenvalues positive)
    double trace = a11 + a22;
    double det = a11 * a22 - a12 * a21;
    
    // Discriminant for eigenvalue formula
    double discriminant = trace * trace - 4.0 * det;
    
    // Critical stability check
    bool stable = true;
    
    // Static stability: C_m_alpha must be negative
    if (C_m_alpha >= 0.0) {
        stable = false;  // Loss of static stability (Mach tuck)
    }
    
    // Dynamic stability: trace < 0 and det > 0
    if (trace >= 0.0) {
        stable = false;  // Positive trace → at least one unstable mode
    }
    
    if (det <= 0.0) {
        stable = false;  // Negative determinant → opposite sign eigenvalues
    }
    
    return stable;
}

void AnalyseurEnveloppeVol::analyser_limites_vitesse() {
    using namespace Physique;
    
    cout << "Alt (m) | Vmax (m/s) | Mach_max | Vmin (m/s) | Mach_min | Stability Limit" << endl;
    
    ofstream csv("vmax_analysis.csv");
    csv << "altitude,v_max_m_s,Mach_max,v_min,Mach_min,stability_criterion\n";
    
    // Debug output file
    ofstream debug_csv("envelope_debug.csv");
    debug_csv << "altitude,v_test,mach,C_m_alpha,x_f,trace,det,is_stable\n";
    
    vector<double> altitudes = {5000, 6000, 7000, 8000, 9000, 10000, 
                                 11000, 12000, 13000, 14000, 15000};
    
    for (double alt : altitudes) {
        avion.z_ref() = alt;
        double rho = avion.get_env().calculer_rho(alt);
        double v_max = 500.0;
        double v_min;
        
        double S = avion.get_aero().get_surface();
        double W = avion.get_masse() * g;
        double alpha_stall_rad = 15.0 * DEG_TO_RAD;
        double CL_max = 5.0 * (alpha_stall_rad - (-0.035)) + 0.44 * (-0.13);
        
        // Lower limit: stall speed
        v_min = std::sqrt(2.0 * W / (rho * S * CL_max));

        const double v_step = 2.0;  // 2 m/s increment
        const double v_low = v_min + 5.0;  // Start slightly above stall
        const double v_high = 450.0;  // Test up to 450 m/s
        
        // Find V_max by scanning from LOW to HIGH speed
        // Continue while stable, stop when unstable
        bool found_vmax = false;
        v_max = v_high;  // Default to max if always stable
        
        for (double v_test = v_low; v_test <= v_high; v_test += v_step) {
            // Calculate trim conditions
            double CL_needed = (2.0 * W) / (rho * v_test * v_test * S);
            if (CL_needed > CL_max) continue;  // Beyond stall
            
            // Find trim elevator and alpha
            double delta_p = avion.trouver_delta_profondeur(v_test, 0);
            double alpha_trim = ((CL_needed - 0.44 * delta_p) / 5.0) + (-0.035);
            
            // Calculate Mach number
            double mach = avion.get_env().calculer_mach(v_test, alt);
            
            // Calculate C_m_alpha for debug
            const double x_f0 = 0.20;
            double x_f_compressibility = 0.0;
            if (mach > 0.3) {
                double k_mach = 0.80;
                x_f_compressibility = k_mach * (mach - 0.3) * (mach - 0.3);
                if (mach > 0.75) {
                    x_f_compressibility += 0.5 * (mach - 0.75);
                }
            }
            double x_f = x_f0 - x_f_compressibility;
            double C_m_alpha_val = -5.0 * x_f;  // NEGATIVE for stability
            
            // Check dynamic stability
            bool is_stable = check_stability(v_test, alpha_trim, mach, rho, delta_p);
            
            // Debug output for selected altitudes
            if (alt >= 11000.0 && (int(v_test) % 10 == 0 || !is_stable)) {
                const double m = avion.get_masse();
                const double c = avion.get_aero().get_corde();
                const double I_y = avion.get_I_pitch();
                double q_dyn = 0.5 * rho * v_test * v_test;
                double Z_alpha = -q_dyn * S * 5.0;
                double M_alpha = q_dyn * S * c * C_m_alpha_val;
                double M_q = 0.25 * rho * v_test * S * c * c * (-12.0);
                double a11 = Z_alpha / (m * v_test);
                double a12 = v_test;
                double a21 = M_alpha / (I_y * v_test);
                double a22 = M_q / I_y;
                double trace = a11 + a22;
                double det = a11 * a22 - a12 * a21;
                
                debug_csv << alt << "," << v_test << "," << mach << "," 
                         << C_m_alpha_val << "," << x_f << "," 
                         << trace << "," << det << "," << (is_stable ? 1 : 0) << "\n";
            }
            
            if (!is_stable) {
                // Found first unstable speed - previous speed is V_max
                v_max = v_test - v_step;
                found_vmax = true;
                cout << "  [DEBUG] Instability at V=" << v_test << " m/s, Mach=" << mach << endl;
                break;
            }
        }
        
        if (!found_vmax) {
            // All tested speeds were stable, use max tested
            v_max = v_high;
        }
        
        // Ensure v_max >= v_min
        if (v_max < v_min) {
            v_max = v_min + 5.0;
        }
        
        double sound_speed = avion.get_env().calculer_vitesse_son(alt);
        double Mach_max = v_max / sound_speed;
        double Mach_min = v_min / sound_speed;
        
        // Determine limiting criterion
        string limit_criterion = (found_vmax) ? "Stability" : "No_limit";
        
        cout << fixed << "\n";
        cout << alt << " | " << v_max << " | " << Mach_max 
             << " | " << v_min << " | " << Mach_min << " | " << limit_criterion << endl;
             
        csv << alt << "," << v_max << "," << Mach_max << "," 
            << v_min << "," << Mach_min << "," << limit_criterion << endl;
    }
    
    csv.close();
    cout << "Résultats sauvegardés dans 'vmax_analysis.csv'" << endl;
}