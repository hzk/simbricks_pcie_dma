#ifndef ACCEL_SIM_PLUMBING_H_
#define ACCEL_SIM_PLUMBING_H_

#ifdef __cplusplus
extern "C" {
#endif

void main_for_CACTUS(int argc, char *argv[]);
typedef void (*readBack)(char * data);
void read_start_for_CACTUS(readBack rb);

#ifdef __cplusplus
}
#endif

#endif  // ndef ACCEL_SIM_PLUMBING_H_
