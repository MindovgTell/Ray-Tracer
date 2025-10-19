#ifndef OBJECTS_HPP
#define OBJECTS_HPP


class Mesh {
    std::vector<glm::vec3> positions;     // size = numVerts
    std::vector<glm::vec3> normals;       // optional
    std::vector<glm::vec2> uvs;           // optional
    std::vector<uint32_t>  indices;       // 3*triangleCount
    int32_t material_index{0};
};

class Sphere {
    public:
        Sphere () : radius(1.0f), center{0.0f, 0.0f, 0.0f}, emission{0.0f, 0.0f, 0.0f} {}
        Sphere (float r, vec3 cent, vec3 emi, std::shared_ptr<Material> m) : 
            radius(r), center(cent), emission(emi), mat(m) {}

        // Getter functions
        const float get_radius()   const       { return radius;    }
        const glm::vec3 get_center_pos()   const   { return center;    }
        const glm::vec3 get_emission()  const  { return emission;  }
        const std::shared_ptr<Material>& get_material_ptr() const {return mat;}


        // Setter functions
        void set_radius(float r)            { radius    = r;    }
        void set_center_pos(glm::vec3 cent) { center    = cent; }
        void set_emission(glm::vec3 emi)    { emission  = emi;  }


    private:
        float radius;
        glm::vec3 center;
        glm::vec3 emission;
        std::shared_ptr<Material> mat;
};

class Scene {

public:
    std::vector<Sphere> spheres;
    std::vector<std::shared_ptr<Material>> materials;
    std::vector<Mesh> meshes;
public:
    void add_sphere(Sphere& sphere) {
        spheres.push_back(sphere);
    }

    void define_sphere_vec_size(size_t size) {
        spheres.reserve(size);
    }

    void set_spheres_vec(const std::vector<Sphere>& vec) {
        spheres = vec;
    }

    void set_materials_vec(const std::vector<std::shared_ptr<Material>>& vec) {
        materials = vec;
    }

    int get_spheres_count() const {return spheres.size();} 
    int get_materials_count() const {return materials.size();}

};


#endif // OBJECTS_HPP