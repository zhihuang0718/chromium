// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_DEVICE_GENERIC_SENSOR_ORIENTATION_UTIL_H_
#define SERVICES_DEVICE_GENERIC_SENSOR_ORIENTATION_UTIL_H_

#include <vector>

namespace device {

// Conversion ratio from radians to degrees.
extern const double kRadToDeg;

// Conversion ratio from degrees to radians.
extern const double kDegToRad;

void ComputeOrientationEulerAnglesFromRotationMatrix(
    const std::vector<double>& r,
    double* alpha_in_degrees,
    double* beta_in_degrees,
    double* gamma_in_degrees);

}  // namespace device

#endif  // SERVICES_DEVICE_GENERIC_SENSOR_ORIENTATION_UTIL_H_
