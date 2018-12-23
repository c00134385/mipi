#ifndef __GPS_ALGORITHM_H__
#define __GPS_ALGORITHM_H__

#include "integer.h"
#include "math.h"

void transform(double wgLat, double wgLon, double *lat, double *lon);

float DistanceCalcs(const double *lat1, const double *lon1, double *lat2, double *lon2);

#endif

