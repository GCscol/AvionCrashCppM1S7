#ifndef LECTURE_NACA_H
#define LECTURE_NACA_H

#include <string>
#include <vector>

class Aerodynamics {
public:
    bool loadPolarCSV(const std::string& filename);
    double getCl(double alpha) const;
    double getCd(double alpha) const;
    double getCm(double alpha) const;

private:
    std::vector<double> alpha_data, Cl_data, Cd_data, Cm_data;
    double interpolate(const std::vector<double>& x,
                       const std::vector<double>& y,
                       double a) const;
};

#endif
