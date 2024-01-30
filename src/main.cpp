#include <SFML/Graphics.hpp>
#include <iostream>

struct triangle {
    sf::Vector3f t[3];
};

struct mesh {
    std::vector<triangle> tris;
};

struct mat4x4 {
    float m[4][4] = { 0 };
};

sf::Color getRandomColor();
void MultiplyMatrixVector(sf::Vector3f& i, sf::Vector3f& o, mat4x4& m);

int main()
{
    sf::Vector2f ScreenDimensions = sf::Vector2f(1200.0f, 1000.0f);
    sf::RenderWindow window(sf::VideoMode(ScreenDimensions.x, ScreenDimensions.y), "SFML works!");
    /*sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);*/
    mesh meshCube;

    meshCube.tris = {
        // SOUTH
        {sf::Vector3f(0.0f, 0.0f, 0.0f),     sf::Vector3f(0.0f, 1.0f, 0.0f),     sf::Vector3f(1.0f, 1.0f, 0.0f)},
        {sf::Vector3f(0.0f, 0.0f, 0.0f),     sf::Vector3f(1.0f, 1.0f, 0.0f),     sf::Vector3f(1.0f, 0.0f, 0.0f)},

        // EAST
        {sf::Vector3f(1.0f, 0.0f, 0.0f),     sf::Vector3f(1.0f, 1.0f, 0.0f),     sf::Vector3f(1.0f, 1.0f, 1.0f)},
        {sf::Vector3f(1.0f, 0.0f, 0.0f),     sf::Vector3f(1.0f, 1.0f, 1.0f),     sf::Vector3f(1.0f, 0.0f, 1.0f)},

        // NORTH
        {sf::Vector3f(1.0f, 0.0f, 1.0f),     sf::Vector3f(1.0f, 1.0f, 1.0f),     sf::Vector3f(0.0f, 1.0f, 1.0f)},
        {sf::Vector3f(1.0f, 0.0f, 1.0f),     sf::Vector3f(0.0f, 1.0f, 1.0f),     sf::Vector3f(0.0f, 0.0f, 1.0f)},

        //// WEST
        {sf::Vector3f(0.0f, 0.0f, 1.0f),     sf::Vector3f(0.0f, 1.0f, 1.0f),     sf::Vector3f(0.0f, 1.0f, 0.0f)},
        {sf::Vector3f(0.0f, 0.0f, 1.0f),     sf::Vector3f(0.0f, 1.0f, 0.0f),     sf::Vector3f(0.0f, 0.0f, 0.0f)},

        //// TOP
        {sf::Vector3f(0.0f, 1.0f, 0.0f),     sf::Vector3f(0.0f, 1.0f, 1.0f),     sf::Vector3f(1.0f, 1.0f, 1.0f)},
        {sf::Vector3f(0.0f, 1.0f, 0.0f),     sf::Vector3f(1.0f, 1.0f, 1.0f),     sf::Vector3f(1.0f, 1.0f, 0.0f)},

        //// BOTTOM
        {sf::Vector3f(1.0f, 0.0f, 1.0f),     sf::Vector3f(0.0f, 0.0f, 1.0f),     sf::Vector3f(0.0f, 0.0f, 0.0f)},
        {sf::Vector3f(1.0f, 0.0f, 1.0f),     sf::Vector3f(0.0f, 0.0f, 0.0f),     sf::Vector3f(1.0f, 0.0f, 0.0f)},


    };

    // Projection Matrix
    float fNear = 0.1f;
    float fFar = 1000.f;
    float fFOV = 90.0f;
    float fAspectRatio = ScreenDimensions.y / ScreenDimensions.x;
    float fFOVRad = 1.0f / tanf(fFOV * 0.5f / 180.0f * 3.14159);

    mat4x4 matProj;

    matProj.m[0][0] = fAspectRatio * fFOVRad;
    matProj.m[1][1] = fFOVRad;
    matProj.m[2][2] = fFar / (fFar - fNear);
    matProj.m[2][3] = (-fFar * fNear) / (fFar - fNear);
    matProj.m[3][2] = 1.0f;
    matProj.m[3][3] = 0.0f;

    mat4x4 matRotZ, matRotX;
    float fTheta = 0;


    

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }



        window.clear();

        for (auto tri : meshCube.tris)
        {
            sf::VertexArray vertexArray(sf::Triangles, 3);

            fTheta += .01f * 0.0003f;
            // Rotation Z
            matRotZ.m[0][0] = cosf(fTheta);
            matRotZ.m[0][1] = sinf(fTheta);
            matRotZ.m[1][0] = -sinf(fTheta);
            matRotZ.m[1][1] = cosf(fTheta);
            matRotZ.m[2][2] = 1;
            matRotZ.m[3][3] = 1;

            // Rotation X
            matRotX.m[0][0] = 1;
            matRotX.m[1][1] = cosf(fTheta * 0.5f);
            matRotX.m[1][2] = sinf(fTheta * 0.5f);
            matRotX.m[2][1] = -sinf(fTheta * 0.5f);
            matRotX.m[2][2] = cosf(fTheta * 0.5f);
            matRotX.m[3][3] = 1;

            triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

            // Rotate in Z-Axis
            MultiplyMatrixVector(tri.t[0], triRotatedZ.t[0], matRotZ);
            MultiplyMatrixVector(tri.t[1], triRotatedZ.t[1], matRotZ);
            MultiplyMatrixVector(tri.t[2], triRotatedZ.t[2], matRotZ);

            // Rotate in X-Axis
            MultiplyMatrixVector(triRotatedZ.t[0], triRotatedZX.t[0], matRotX);
            MultiplyMatrixVector(triRotatedZ.t[1], triRotatedZX.t[1], matRotX);
            MultiplyMatrixVector(triRotatedZ.t[2], triRotatedZX.t[2], matRotX);

            // Offset into the screen
            triTranslated = triRotatedZX;
            triTranslated.t[0].z = triRotatedZX.t[0].z - 30.0f;
            triTranslated.t[1].z = triRotatedZX.t[1].z - 30.0f;
            triTranslated.t[2].z = triRotatedZX.t[2].z - 30.0f;
            
            // Project triangles from 3D --> 2D
            MultiplyMatrixVector(triTranslated.t[0], triProjected.t[0], matProj);
            MultiplyMatrixVector(triTranslated.t[1], triProjected.t[1], matProj);
            MultiplyMatrixVector(triTranslated.t[2], triProjected.t[2], matProj);

            // Scale into view
            triProjected.t[0].x += 1.0f;
            triProjected.t[0].y += 1.0f;

            triProjected.t[1].x += 1.0f;
            triProjected.t[1].y += 1.0f;

            triProjected.t[2].x += 1.0f;
            triProjected.t[2].y += 1.0f;

            triProjected.t[0].x *= 0.5f * ScreenDimensions.x;
            triProjected.t[0].y *= 0.5f * ScreenDimensions.y;

            triProjected.t[1].x *= 0.5f * ScreenDimensions.x;
            triProjected.t[1].y *= 0.5f * ScreenDimensions.y;

            triProjected.t[2].x *= 0.5f * ScreenDimensions.x;
            triProjected.t[2].y *= 0.5f * ScreenDimensions.y;

            vertexArray[0].position = sf::Vector2f(triProjected.t[0].x, triProjected.t[0].y);
            vertexArray[0].color = sf::Color::Red;

            vertexArray[1].position = sf::Vector2f(triProjected.t[1].x, triProjected.t[1].y);
            vertexArray[1].color = sf::Color::Green;

            vertexArray[2].position = sf::Vector2f(triProjected.t[2].x, triProjected.t[2].y);
            vertexArray[2].color = sf::Color::Blue;

            window.draw(vertexArray);
        }

        window.display();
    }

    return 0;
}

void MultiplyMatrixVector(sf::Vector3f& i, sf::Vector3f& o, mat4x4& m)
{
    o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
    o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
    o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
    float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

    if (w != 0.0f)
    {
        o.x /= w;
        o.y /= w;
        o.z /= w;
    }
}

sf::Color getRandomColor() {
    return sf::Color(rand() % 256, rand() % 256, rand() % 256);
}