#include "pchray.h"
#include "CLBackend.hpp"


namespace compute {

    std::unique_ptr<Backend> CreateBackend(BackendType type) {
        switch (type) {
            case BackendType::OpenCL:
                return std::make_unique<CLBackend>();
            // Adding other backends in the future
            // case BackendType::Metal:
            //     throw std::runtime_error("Metal backend not implemented.");
            // case BackendType::Vulkan:
            //     throw std::runtime_error("Vulkan backend not implemented.");
            // case BackendType::CUDA:
            //     throw std::runtime_error("CUDA backend not implemented.");
            default:
                throw std::runtime_error("Unknown backend type.");
        }
    }

}