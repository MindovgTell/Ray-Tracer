#ifndef PCHRAY_H
#define PCHRAY_H

#include <cmath>
#include <random>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>


#include <glm/glm.hpp>

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

// Common Headers
#include "Camera.hpp"
#include "Objects.hpp"

#endif // PCHRAY_H
