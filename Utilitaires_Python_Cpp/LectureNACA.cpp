#include "LectureNACA.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

#include "LectureNACA.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

bool Aerodynamics::loadPolarCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur ouverture fichier " << filename << std::endl;
        return false;
    }

    std::string line;
    bool headerFound = false;

    alpha_data.clear();
    Cl_data.clear();
    Cd_data.clear();
    Cm_data.clear();

    while (std::getline(file, line)) {
        // Remove carriage return for Windows files
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        // Trim leading/trailing spaces
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());

        if (line.empty()) continue;

        // Find the header line
        if (!headerFound) {
            if (line.find("Alpha") != std::string::npos &&
                line.find("Cl") != std::string::npos) {
                headerFound = true;
            }
            continue;
        }

        // Parse CSV columns using getline with ',' delimiter
        std::stringstream ss(line);
        std::string token;
        double alpha, Cl, Cd, Cdp, Cm;

        try {
            std::getline(ss, token, ','); alpha = std::stod(token);
            std::getline(ss, token, ','); Cl    = std::stod(token);
            std::getline(ss, token, ','); Cd    = std::stod(token);
            std::getline(ss, token, ','); Cdp   = std::stod(token); // ignored
            std::getline(ss, token, ','); Cm    = std::stod(token);
        } catch (const std::exception& e) {
            // Skip lines that cannot be parsed
            continue;
        }

        // Skip lines where all coefficients are zero
        if (Cl == 0.0 && Cd == 0.0 && Cm == 0.0) continue;

        alpha_data.push_back(alpha);
        Cl_data.push_back(Cl);
        Cd_data.push_back(Cd);
        Cm_data.push_back(Cm);
    }

    file.close();

    if (alpha_data.empty()) {
        std::cerr << "Aucune donnée lue." << std::endl;
        return false;
    }

    // Ensure angles are in ascending order
    if (alpha_data.front() > alpha_data.back()) {
        std::reverse(alpha_data.begin(), alpha_data.end());
        std::reverse(Cl_data.begin(), Cl_data.end());
        std::reverse(Cd_data.begin(), Cd_data.end());
        std::reverse(Cm_data.begin(), Cm_data.end());
    }

    //std::cout << "Fichier " << filename << " lu : "<< alpha_data.size() << " points (" << alpha_data.front() << " à " << alpha_data.back() << "°)\n";

    return true;
}


double Aerodynamics::interpolate(const std::vector<double>& x,
                                 const std::vector<double>& y,
                                 double a) const {
    if (x.empty()) return 0.0;
    if (a <= x.front()) return y.front();
    if (a >= x.back())  return y.back();

    auto it = std::lower_bound(x.begin(), x.end(), a);
    size_t i = std::distance(x.begin(), it);
    if (i == 0) i = 1;

    double t = (a - x[i-1]) / (x[i] - x[i-1]);
    return y[i-1] + t * (y[i] - y[i-1]);
}

double Aerodynamics::getCl(double alpha) const { return interpolate(alpha_data, Cl_data, alpha); }
double Aerodynamics::getCd(double alpha) const { return interpolate(alpha_data, Cd_data, alpha); }
double Aerodynamics::getCm(double alpha) const { return interpolate(alpha_data, Cm_data, alpha); }
