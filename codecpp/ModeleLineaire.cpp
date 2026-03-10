#include "ModeleLineaire.h"
#include "Constantes.h"
#include <cmath>
#include <algorithm>

ModeleLineaire::ModeleLineaire(double surface, double corde, double delta_prof_init)
    : ModeleAerodynamique(surface, corde, delta_prof_init) {}

ModeleLineaire::~ModeleLineaire() {}


void ModeleLineaire::update_from_polar(double alpha_rad, double delta_p, 
                                       double omega, double vitesse, double mach) {
    using namespace Math;
    
    // Parabolic stall model with smooth tangent transition
    // Linear region: alpha <= 13°
    // Parabolic post-stall region: alpha > 13°
    
    const double alpha0 = -0.035;  // zero-lift angle in radians
    const double alpha_tangent_deg = 13.0;  // transition point (degrees)
    const double alpha_peak_deg = 15.0;  // peak lift angle (degrees)
    const double alpha_tangent_rad = alpha_tangent_deg * DEG_TO_RAD;
    const double alpha_peak_rad = alpha_peak_deg * DEG_TO_RAD;
    const double alpha_tangent_neg_deg = -4.0;
    const double alpha_tangent_neg_rad = alpha_tangent_neg_deg * DEG_TO_RAD;
    
    // Parabola coefficient: ensures tangency at 13° and maximum at 15°
    // Corrected to eliminate discontinuity
    const double A = -71.6197;  // coefficient of quadratic term
    const double C_L_peak = 1.3967;  // C_L maximum at alpha_peak
    
    // Linear slope and elevator authority
    const double slope = 5.0;
    const double elevator_auth = 0.44;
    
    // Compute attached-flow lift
    double C_L_attached = slope * (alpha_rad - alpha0) + elevator_auth * delta_p;
    
    if (alpha_rad <= alpha_tangent_rad) {
        // Attached flow region (linear)
        C_L = C_L_attached;
    } else {
        // Post-stall region: parabolic decay with delta_p continuity offset
        // C_L = A*(alpha - alpha_peak)^2 + C_L_peak + elevator_auth*delta_p
        double alpha_offset = alpha_rad - alpha_peak_rad;
        double parabolic = A * alpha_offset * alpha_offset + C_L_peak + elevator_auth * delta_p;
        // Clamp to non-negative (C_L cannot be negative at high alpha)
        C_L = std::max(0.0, parabolic);
    }
    
    // Drag: induced + 2*sin^2(alpha) post-stall with continuity at -4 and 13 deg
    const double cd0 = 0.0175;
    const double cd_induced = 0.055;
    if (alpha_rad <= alpha_tangent_neg_rad) {
        double C_L_neg = slope * (alpha_tangent_neg_rad - alpha0) + elevator_auth * delta_p;
        double C_D_neg = cd0 + cd_induced * C_L_neg * C_L_neg;
        double sin2_delta = std::sin(alpha_rad) * std::sin(alpha_rad)
            - std::sin(alpha_tangent_neg_rad) * std::sin(alpha_tangent_neg_rad);
        C_D = C_D_neg + 2.0 * sin2_delta;
    } else if (alpha_rad >= alpha_tangent_rad) {
        double C_L_pos = slope * (alpha_tangent_rad - alpha0) + elevator_auth * delta_p;
        double C_D_pos = cd0 + cd_induced * C_L_pos * C_L_pos;
        double sin2_delta = std::sin(alpha_rad) * std::sin(alpha_rad)
            - std::sin(alpha_tangent_rad) * std::sin(alpha_tangent_rad);
        C_D = C_D_pos + 2.0 * sin2_delta;
    } else {
        C_D = cd0 + cd_induced * C_L * C_L;
    }
    
    // Pitching moment with damping (with velocity safety check)
    const double Cz_alpha = 5.0;
    const double x_f0 = 0.20;  // static margin (x_f0 - x_cg) / c at low Mach
    
    // Compressibility effects: aerodynamic center shifts aft → REDUCES static margin
    // At high Mach, if AC moves behind CG, C_m becomes positive (pitch-up tendency)
    double x_f_compressibility = 0.0;
    if (mach > 0.3) {
        // Static margin reduction due to aft AC shift in transonic regime
        // At M=0.85, margin can be reduced by ~50% for transport aircraft
        double k_mach = 0.80;  // Empirical coefficient (stronger than before)
        x_f_compressibility = k_mach * (mach - 0.3) * (mach - 0.3);
        
        // Critical Mach effect: beyond M_crit, C_m can become positive
        if (mach > 0.78) {
            // Enhanced destabilization near transonic regime
            x_f_compressibility += 0; // 0.5 * (mach - 0.78);
        }
    }
    
    // Static margin DECREASES with Mach (can become negative → unstable)
    double x_f = x_f0; //- x_f_compressibility;
    double cm_alpha = Cz_alpha * x_f;
    double vitesse_safe = std::max(vitesse, 1.0);
    C_m = -0.1 - cm_alpha * (alpha_rad - alpha0) - 1.46 * delta_p - 12.0 * omega * l / vitesse_safe;
}