#ifndef OBJECTS_HPP
#define OBJECTS_HPP

class Sphere {
    public:
        float radius;
        float position[3];
        float color[3];
        float emission[3];
        Sphere () : radius(1.0f), position{0.0f, 0.0f, 0.0f}, color{1.0f, 1.0f, 1.0f}, emission{0.0f, 0.0f, 0.0f} {}
};

class Scene {

private:
    std::vector<Sphere> spheres;

public:
    void addSphere(const Sphere& sphere) {
        spheres.push_back(sphere);
    }

};

#endif // OBJECTS_HPP