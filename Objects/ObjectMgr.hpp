#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <strstream>

struct vec3D {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 2; // Need a 4th term for matrix vector multiplication
};

struct Triangle {
    vec3D t[3];
};

struct Mesh {
    std::vector<Triangle> tris;

    bool loadFromObjFile(std::string sFilename)
    {
        std::ifstream f(sFilename);
        if (!f.is_open())
        {
            std::cout << "File wasn't found" << std::endl;
            return false;

        }

        // Local cache of verts
        std::vector<vec3D> verts;

        while (!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            std::strstream s;
            s << line;

            char junk;

            if (line[0] == 'v')
            {
                vec3D v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }

            if (line[0] == 'f')
            {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];

                // All of the info in an obj file starts counting from 1, not 0 (therefore - 1)
                tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
            }
        }

        return true;
    }
};