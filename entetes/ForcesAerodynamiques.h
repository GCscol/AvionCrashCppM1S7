#ifndef FORCES_AERODYNAMIQUES_H
#define FORCES_AERODYNAMIQUES_H

struct ForcesAerodynamiques {
    double portance = 0.0;
    double trainee = 0.0;
    double traction = 0.0;
    double poids = 0.0;
    double M_pitch = 0.0;
    double Fx = 0.0;
    double Fy = 0.0;
    double Fz = 0.0;
};

#endif // FORCES_AERODYNAMIQUES_H
