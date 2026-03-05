#ifndef BATCH_RUNNER_H
#define BATCH_RUNNER_H

int run_batch(double p_min, double p_max, double p_step,
              double t_min, double t_max, double t_step,
              bool useHysteresis,
              double sim_dt = 0.01, double sim_duration = 600.0,
              double cmd_start = 100.0, double cmd_end = 500.0);

#endif