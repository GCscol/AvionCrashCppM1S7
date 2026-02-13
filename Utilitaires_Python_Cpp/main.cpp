#include "LectureNACA.cpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
using namespace std;

double g = 9.81, Cl, Cd, Cm;
double rho=0.4135; // densité de l'air
double L = 6; // Corde A330
double Taille_fuselage = 58.80; // Longueur du fuselage
double S = 361.6 , S_empennage = 122; // Surfaces alaire des ailes et de l'empennage
double m = 2.00e5; // masse A330
double Jy = 1.7e7; // Moment d'inertie du fuselage (vu comme un cylindre creux)
double alpha;

void sda(const vector<double>& q, double t, vector<double>& qp) {
    double Thrust = 2*3e5; // poussée totale
    double vx = q[3], vy = q[4];
    double V = sqrt(vx*vx + vy*vy);
    double gamma = atan2(vy, vx); // angle de trajectoire
    double theta = q[2];          // assiette

    qp[0] = vx;       // dx/dt
    qp[1] = vy;       // dy/dt
    qp[2] = q[5];     // dtheta/dt
    alpha = theta - gamma; // angle d’attaque

    // Forces aérodynamiques 𝐶𝑧= 𝐶𝑧𝛼 (𝛼−𝛼0)+ 𝐶𝑧𝛿𝑚 𝛿𝑚 + Δ𝐶𝑧𝑆𝑃
    double F_lift = 0.5 * (5 * (alpha+0.035) + 0.44*0.00)  * rho * S * V*V;
    double F_drag = 0.5 * Cd * rho * S * V*V;

    // Projection dans le repère terrestre
    double Fx_drag = -F_drag * cos(gamma);
    double Fy_drag = -F_drag * sin(gamma);

    double Fx_lift = -F_lift * sin(gamma);
    double Fy_lift =  F_lift * cos(gamma);

    // Poussée
    double Fx_thrust = Thrust * cos(theta);
    double Fy_thrust = Thrust * sin(theta);

    // Poids
    double Fy_weight = -m * g;

    // Équations du mouvement
    qp[3] = (Fx_drag + Fx_lift + Fx_thrust) / m;  // accel x
    qp[4] = (Fy_drag + Fy_lift + Fy_thrust + Fy_weight) / m;  // accel y

    cout << "qp4 = " << qp[4] << " V = " << V << " F = " << 0.5 * Cl * rho * S * V*V / (m*g) << " z = " << q[1] << " theta = " <<  theta << " alpha = " << alpha << endl;

    // Moment (simplifié)
    qp[5] = ( -0.5 * Cm * L * rho * S * V*V
              - (Taille_fuselage/2) * 0.5 * Cl * rho * S_empennage * V*V * cos(gamma) ) / Jy;
}

void rk4_step(void(*sd)(const vector<double>&, double, vector<double>&), vector<double>& q, double t, double dt) {
    size_t n = q.size();
    vector<double> k1(n), k2(n), k3(n), k4(n), qt(n);

    sd(q, t, k1);
    for (size_t i=0;i<n;i++) qt[i] = q[i] + 0.5*dt*k1[i];
    sd(qt, t+0.5*dt, k2);
    for (size_t i=0;i<n;i++) qt[i] = q[i] + 0.5*dt*k2[i];
    sd(qt, t+0.5*dt, k3);
    for (size_t i=0;i<n;i++) qt[i] = q[i] + dt*k3[i];
    sd(qt, t+dt, k4);
    for (size_t i=0;i<n;i++) q[i] += dt*(k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i]) / 6.0;
}

int main() {
        Aerodynamics aero;
    if (!aero.loadPolarCSV("xf-naca23012-il-1000000.csv")) {
        std::cerr << "Erreur lecture fichier polar" << std::endl;
        return -1;
    }
    int np = 1e4;
    double tfin = 200;
    double dt = tfin/(np-1);
        vector<double> q(6);
    q[0] = 0;       // x
    q[1] = 10000;   // y
    q[2] =  1 * M_PI / 180.0; // theta, ou alpha
    q[3] = 240;     // vx
    q[4] = 0;       // vy
    q[5] = 0;
    ofstream res("rk4_1.res");
        for(int i=0; i<np; i++) {
        double alpha_deg = alpha * 180.0 / M_PI;
        if (alpha_deg < -15.25 || alpha_deg > 18.0) {
            std::cout << "Attention  alpha hors domaine : " << alpha_deg << "°" << std::endl;
        }
        Cl = aero.getCl(alpha_deg);
        Cd = aero.getCd(alpha_deg);
        Cm = aero.getCm(alpha_deg);
        //cout << " Cm = " << Cm << endl;

        res << i*dt << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << " " << q[4] << " " << q[5] << endl;
        //cout << Cl << " " << Cd << " " << Cm << endl;
        //cout << "alpha =  " << alpha_deg << " theta = " << q[2] * 180/M_PI << " rapport Portance/Poids =  " << 0.5 * Cl * rho * S * (pow(q[3],2) + pow(q[4],2)) / (m * g) << endl ;//" " << q[3] << " " << q[4] << " " << q[5] << endl;

        rk4_step(sda, q, i*dt, dt);
    }

    res.close();
    return 0;
}
