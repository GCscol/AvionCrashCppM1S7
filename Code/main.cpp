#include "LectureNACA.cpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
using namespace std;

double g = 9.81, Cl, Cd, Cm, rho=1.225, L = 6, Taille_fuselage = 58.80 , S = 361.6 , S_empennage = 122,  m=200e3, Jy = 1.7e7; // L: Corde A330


void sda(const vector<double>& q, double t, vector<double>& qp) {
    double Thrust = 3e5*2; // Poussée de l'ordre de 300 kN
    qp[0] = q[3]; // dx/dt = vx
    qp[1] = q[4]; // dy/dt = vy
    qp[2] = q[5]; // dtheta/dt = omega
    qp[3] = -0.5 * Cd * rho * S * sin(q[2]) * (pow(q[3],2) + pow(q[4],2)) / m  + Thrust * cos(q[2]) / m; // accel x
    qp[4] = 0.5 * Cl * rho * S * cos(q[2])  * (pow(q[3],2) + pow(q[4],2)) / m + Thrust * sin(q[2]) / m - g; // accel y
    qp[5] = 1 / Jy * (0.5 * Cm * L * rho * S * cos(q[2]) * (pow(q[3],2)+pow(q[4],2)) - 0.5 * Taille_fuselage * cos(q[2])* 0.5 * Cl * rho * S_empennage * cos(q[2])  * (pow(q[3],2) + pow(q[4],2)));
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
    double tfin = 300;
    double dt = tfin/(np-1);
        vector<double> q(6);
    q[0] = 0;       // x
    q[1] = 10000;   // y
    q[2] = 0 * M_PI / 180.0; // theta, ou alpha
    q[3] = 240;     // vx
    q[4] = 0;       // vy
    q[5] = 0;
    ofstream res("rk4_1.res");
        for(int i=0; i<np; i++) {
        double alpha_deg = q[2] * 180.0 / M_PI;
        if (alpha_deg < -15.25 || alpha_deg > 18.0) {
            std::cout << "⚠️  alpha hors domaine : " << alpha_deg << "°" << std::endl;
        }
        Cl = aero.getCl(alpha_deg);
        Cd = aero.getCd(alpha_deg);
        Cm = aero.getCm(alpha_deg);
        //cout << " Cm = " << Cm << endl;

        res << i*dt << " " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << " " << q[4] << " " << q[5] << endl;
        //cout << Cl << " " << Cd << " " << Cm << endl;
        //cout << "alpha =  " << alpha_deg << "rapport Portance/Poids =  " << 0.5 * Cl * rho * S * cos(q[2])  * (pow(q[3],2) + pow(q[4],2)) / (m * g) << endl ;//" " << q[3] << " " << q[4] << " " << q[5] << endl;

        rk4_step(sda, q, i*dt, dt);
    }

    res.close();
    return 0;
}
