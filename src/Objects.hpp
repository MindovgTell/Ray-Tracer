#ifndef OBJECTS_HPP
#define OBJECTS_HPP

class Sphere {
    public:
        Sphere () : radius(1.0f), center{0.0f, 0.0f, 0.0f}, color{1.0f, 1.0f, 1.0f}, emission{0.0f, 0.0f, 0.0f} {}

        // Getter functions
        float get_radius()          { return radius;    }
        glm::vec3 get_center()      { return center;    }
        glm::vec3 get_color()       { return color;     }
        glm::vec3 get_emission()    { return emission;  }


        // Setter functions
        void set_radius(float r)            { radius    = r;    }
        void set_center(glm::vec3 cent)     { center    = cent; }
        void set_color(glm::vec3 col)       { color     = col;  }
        void set_emission(glm::vec3 emi)    { emission  = emi;  }


    private:
        float radius;
        glm::vec3 center;
        glm::vec3 color;
        glm::vec3 emission;
};

class Scene {

private:
    std::vector<Sphere> spheres;
    std::vector<std::shared_ptr<Material>> materials
public:
    void add_sphere(Sphere& sphere) {
        spheres.push_back(sphere);
    }

};

#endif // OBJECTS_HPP