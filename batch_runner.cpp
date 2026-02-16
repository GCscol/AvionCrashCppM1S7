#include "Avion.h"
#include "Simulateur.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

int run_batch(double p_min, double p_max, double p_step,
              double t_min, double t_max, double t_step,
              bool useHysteresis,
              double sim_dt = 0.01, double sim_duration = 600.0,
              double cmd_start = 100.0, double cmd_end = 500.0) {

    std::vector<double> elevs;
    std::vector<double> thrusts;
    if (p_step == 0.0 || t_step == 0.0) return 1;
    for (double d = p_min; d <= p_max + 1e-12; d += p_step) elevs.push_back(d);
    for (double tt = t_min; tt <= t_max + 1e-12; tt += t_step) thrusts.push_back(tt);

    std::string resname = std::string("output/batch_results_") + (useHysteresis ? "hyst.csv" : "lin.csv");
    std::ofstream out(resname);
    out << "cmd_profondeur,cmd_thrust,crash,crash_time,final_altitude\n";

    for (double cmd_p : elevs) {
        for (double cmd_t : thrusts) {
            Avion avion(361.6, 6.6, 140178.9, useHysteresis);
            avion.initialiser();

            std::ostringstream fname;
            fname << "output/sim_p" << std::fixed << std::setprecision(2) << cmd_p
                  << "_t" << std::fixed << std::setprecision(2) << cmd_t
                  << (useHysteresis ? "_hyst.csv" : "_lin.csv");

            Simulateur sim(avion, sim_dt, sim_duration, fname.str(), cmd_p, cmd_t, cmd_start, cmd_end);
            double crash_time = sim.executer();

            // read last altitude from the file to report final altitude
            double final_alt = std::numeric_limits<double>::quiet_NaN();
            std::ifstream in(fname.str());
            if (in.is_open()) {
                std::string line;
                std::string last;
                while (std::getline(in, line)) {
                    if (!line.empty()) last = line;
                }
                in.close();
                if (!last.empty()) {
                    // parse CSV last column altitude is 4th column (x,y,z,vz?) in our header altitude is 3rd printed after x,y,z? we wrote x,y,z,vx,vy,vz, so altitude is third printed value -> field index 3 (0-based: time=0,x=1,y=2,z=3)
                    std::stringstream ss(last);
                    std::string item;
                    int idx = 0;
                    while (std::getline(ss, item, ',')) {
                        if (idx == 3) {
                            try { final_alt = std::stod(item); } catch(...) { final_alt = std::numeric_limits<double>::quiet_NaN(); }
                            break;
                        }
                        ++idx;
                    }
                }
            }

            int crash_flag = std::isnan(crash_time) ? 0 : 1;
            out << cmd_p << ',' << cmd_t << ',' << crash_flag << ',';
            if (crash_flag) out << crash_time << ','; else out << ',';
            out << final_alt << '\n';
            out.flush();

            std::cout << "Run p=" << cmd_p << " t=" << cmd_t << " -> crash=" << crash_flag << "\n";
        }
    }

    out.close();
    std::cout << "Batch finished. Results in " << resname << std::endl;
    return 0;
}

// Backwards-compatible no-arg wrapper with sensible defaults
int run_batch() {
    return run_batch(-1.0, 0.0, 0.1,
                     0.0, 1.0, 0.1,
                     true, // use hysteresis by default
                     0.01, 600.0,
                     100.0, 500.0);
}
