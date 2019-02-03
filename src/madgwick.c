// Below is a minimally modified version of the code written by Sebastian O.H. Madgwick
//     from his paper "An efficient orientation filter for inertial and inertial/magnetic sensor arrays"

#include <math.h>

// System constants
#define gyroMeasError 3.14159265358979f * (5.0f / 180.0f) // gyroscope measurement error in rad/s (shown as 5 deg/s)
#define beta sqrt(3.0f / 4.0f) * gyroMeasError // compute beta

// quaternion
struct Q {
    float q1;
    float q2;
    float q3;
    float q4;
};

// vector in 3d space
struct V {
    float x;
    float y;
    float z;
};

// a - accelerometer measurements
// w - gyroscrope measurements in rad/s
// q - orientation quaternion elements initial conditions
struct Q filterUpdateC(struct V w, struct V a, struct Q q, float deltat) {
    // Local system variables
    float norm; // vector norm
    float SEqDot_omega_1, SEqDot_omega_2, SEqDot_omega_3, SEqDot_omega_4; // quaternion derivative from gyroscopes elements
    float f_1, f_2, f_3; // objective function elements
    float J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33; // objective function Jacobian elements
    float SEqHatDot_1, SEqHatDot_2, SEqHatDot_3, SEqHatDot_4; // estimated direction of the gyroscope error

    // Auxiliary variables to avoid reapeated calcualtions
    float halfSEq_1 = 0.5f * q.q1;
    float halfSEq_2 = 0.5f * q.q2;
    float halfSEq_3 = 0.5f * q.q3;
    float halfSEq_4 = 0.5f * q.q4;
    float twoSEq_1 = 2.0f * q.q1;
    float twoSEq_2 = 2.0f * q.q2;
    float twoSEq_3 = 2.0f * q.q3;

    // Normalise the accelerometer measurement
    norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    a.x /= norm;
    a.y /= norm;
    a.z /= norm;

    // Compute the objective function and Jacobian
    f_1 = twoSEq_2 * q.q4 - twoSEq_1 * q.q3 - a.x;
    f_2 = twoSEq_1 * q.q2 + twoSEq_3 * q.q4 - a.y;
    f_3 = 1.0f - twoSEq_2 * q.q2 - twoSEq_3 * q.q3 - a.z;
    J_11or24 = twoSEq_3; // J_11 negated in matrix multiplication
    J_12or23 = 2.0f * q.q4;
    J_13or22 = twoSEq_1; // J_12 negated in matrix multiplication
    J_14or21 = twoSEq_2;
    J_32 = 2.0f * J_14or21; // negated in matrix multiplication
    J_33 = 2.0f * J_11or24; // negated in matrix multiplication

    // Compute the gradient (matrix multiplication)
    SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1;
    SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32 * f_3;
    SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22 * f_1;
    SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2;

    // Normalise the gradient
    norm = sqrt(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
    SEqHatDot_1 /= norm;
    SEqHatDot_2 /= norm;
    SEqHatDot_3 /= norm;
    SEqHatDot_4 /= norm;

    // Compute the quaternion derrivative measured by gyroscopes
    SEqDot_omega_1 = -halfSEq_2 * w.x - halfSEq_3 * w.y - halfSEq_4 * w.z;
    SEqDot_omega_2 = halfSEq_1 * w.x + halfSEq_3 * w.z - halfSEq_4 * w.y;
    SEqDot_omega_3 = halfSEq_1 * w.y - halfSEq_2 * w.z + halfSEq_4 * w.x;
    SEqDot_omega_4 = halfSEq_1 * w.z + halfSEq_2 * w.y - halfSEq_3 * w.x;

    // Compute then integrate the estimated quaternion derrivative
    q.q1 += (SEqDot_omega_1 - (beta * SEqHatDot_1)) * deltat;
    q.q2 += (SEqDot_omega_2 - (beta * SEqHatDot_2)) * deltat;
    q.q3 += (SEqDot_omega_3 - (beta * SEqHatDot_3)) * deltat;
    q.q4 += (SEqDot_omega_4 - (beta * SEqHatDot_4)) * deltat;

    // Normalise quaternion
    norm = sqrt(q.q1 * q.q1 + q.q2 * q.q2 + q.q3 * q.q3 + q.q4 * q.q4);
    q.q1 /= norm;
    q.q2 /= norm;
    q.q3 /= norm;
    q.q4 /= norm;

    return q;
}
