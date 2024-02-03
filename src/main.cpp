#include "../Objects/ObjectMgr.hpp"
#include "../Util/Math.hpp"
#include <iostream>
#include <algorithm>
#include <list>

void populateMesh(Mesh& mesh);

const sf::Vector2f ScreenDimensions = sf::Vector2f(1200.0f, 1000.0f);

sf::Color getColor(float& dp);

int main()
{
    sf::RenderWindow window(sf::VideoMode(ScreenDimensions.x, ScreenDimensions.y), "3D Game Engine 1.3");

    Mesh mesh;
    mat4x4 matProj;
    Math math;
    ObjectMgr objMgr;

    mesh.loadFromObjFile("Objects/mountains.obj");
    //objMgr.createPyramidMesh(mesh);

    // Projection Matrix
    matProj = math.Matrix_MakeProjection(90.0f, (float)ScreenDimensions.y / (float)ScreenDimensions.x, 0.1f, 1000.0f);

    mat4x4 matWorld, matTrans, matRotZ, matRotX;
    float fTheta = 0; 

    matTrans = math.Matrix_MakeTranslation(0.0f, 0.0f, 5.0f);
    
    vec3D vCamera;
    vec3D vLookDir;

    float fYaw = 0;

    bool drawWireFrames = false;

    while (window.isOpen())
    {
        sf::Event event;

        vec3D vForward = math.Vector_Mul(vLookDir, 0.2f);

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            {
                vCamera.y += 0.1f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            {
                vCamera.y -= 0.1f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                fYaw += 0.03f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                fYaw -= 0.03f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                vCamera.x -= 0.1f;
            }
            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                vCamera.x += 0.1f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                vCamera = math.Vector_Add(vCamera, vForward);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                vCamera = math.Vector_Sub(vCamera, vForward);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
            {
                drawWireFrames = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
            {
                drawWireFrames = false;
            }
        }

        const sf::Color CLEAR_COLOR(81,94,60);
        window.clear(CLEAR_COLOR);

        vec3D vUp = { 0, 1, 0 };
        vec3D vTarget = { 0,0,1 };
        mat4x4 matCameraRot = math.Matrix_MakeRotationY(fYaw);
        vLookDir = math.Matrix_MultiplyVector(matCameraRot, vTarget);
        vTarget = math.Vector_Add(vCamera, vLookDir);

        mat4x4 matCamera = math.Matrix_PointAt(vCamera, vTarget, vUp);

        // Make view matrix from camera
        mat4x4 matView = math.Matrix_QuickInverse(matCamera);

        std::vector<Triangle> vecTrianglesToRaster;

        // fTheta += .01f * 0.0003f;
        matRotZ = math.Matrix_MakeRotationZ(fTheta);
        matRotX = math.Matrix_MakeRotationX(fTheta);

        matWorld = math.Matrix_MakeIdentity();
        matWorld = math.Matrix_MultiplyMatrix(matRotZ, matRotX);
        matWorld = math.Matrix_MultiplyMatrix(matWorld, matTrans);

        for (auto tri : mesh.tris)
        {
            Triangle triProjected, triTransformed, triViewed;

            // World Matrix Transform
            triTransformed.t[0] = math.Matrix_MultiplyVector(matWorld, tri.t[0]);
            triTransformed.t[1] = math.Matrix_MultiplyVector(matWorld, tri.t[1]);
            triTransformed.t[2] = math.Matrix_MultiplyVector(matWorld, tri.t[2]);

            // Calc triangle normal
            vec3D normal, line1, line2;

            // Get lines either side of triangle
            line1 = math.Vector_Sub(triTransformed.t[1], triTransformed.t[0]);
            line2 = math.Vector_Sub(triTransformed.t[2], triTransformed.t[0]);

            // Take cross product of lines to get normal to triangle surface
            normal = math.Vector_CrossProduct(line1, line2);

            // normalize normal :P
            normal = math.Vector_Normalise(normal);

            // normal = math.Vector_Mul(normal, -1);

            // Get ray from triangle to camera
            vec3D vCameraRay = math.Vector_Sub(triTransformed.t[0], vCamera);

            if (math.Vector_DotProduct(normal, vCameraRay) < 0.0f)
            {
                // Illumination
                vec3D lightDirection = { 0.0f, 1.0f, -1.0f };
                lightDirection = math.Vector_Normalise(lightDirection);
                lightDirection = math.Vector_Div(lightDirection, 1);

                // How "aligned" are light direction and triangle surface normal?
                float dp = std::max(0.1f, math.Vector_DotProduct(lightDirection, normal));

                vec3D vOffsetView = { 1,1,0 };

                // Convert World Space --> View Space
                triViewed.t[0] = math.Matrix_MultiplyVector(matView, triTransformed.t[0]);
                triViewed.t[1] = math.Matrix_MultiplyVector(matView, triTransformed.t[1]);
                triViewed.t[2] = math.Matrix_MultiplyVector(matView, triTransformed.t[2]);

                // Clip Viewed Triangle against near plane, this could form two additional triangles
                int nClippedTriangles = 0;
                Triangle clipped[2];
                nClippedTriangles = math.Triangle_ClipAgainstPlane(
                    { 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.f }, triViewed, clipped[0], clipped[1]);

                for (int n = 0; n < nClippedTriangles; n++)
                {

                    // Project triangles from 3D --> 2D
                    triProjected.t[0] = math.Matrix_MultiplyVector(matProj, clipped[n].t[0]);
                    triProjected.t[1] = math.Matrix_MultiplyVector(matProj, clipped[n].t[1]);
                    triProjected.t[2] = math.Matrix_MultiplyVector(matProj, clipped[n].t[2]);

                    // Scale into view
                    triProjected.t[0] = math.Vector_Div(triProjected.t[0], triProjected.t[0].w);
                    triProjected.t[1] = math.Vector_Div(triProjected.t[1], triProjected.t[1].w);
                    triProjected.t[2] = math.Vector_Div(triProjected.t[2], triProjected.t[2].w);

                    // X/Y are inverted so put them back
                    triProjected.t[0].x *= -1.0f;
                    triProjected.t[1].x *= -1.0f;
                    triProjected.t[2].x *= -1.0f;
                    triProjected.t[0].y *= -1.0f;
                    triProjected.t[1].y *= -1.0f;
                    triProjected.t[2].y *= -1.0f;

                    // Offset verts into visible normalised space
                    triProjected.t[0] = math.Vector_Add(triProjected.t[0], vOffsetView);
                    triProjected.t[1] = math.Vector_Add(triProjected.t[1], vOffsetView);
                    triProjected.t[2] = math.Vector_Add(triProjected.t[2], vOffsetView);
                    triProjected.t[0].x *= 0.5f * (float)ScreenDimensions.x;
                    triProjected.t[0].y *= 0.5f * (float)ScreenDimensions.y;
                    triProjected.t[1].x *= 0.5f * (float)ScreenDimensions.x;
                    triProjected.t[1].y *= 0.5f * (float)ScreenDimensions.y;
                    triProjected.t[2].x *= 0.5f * (float)ScreenDimensions.x;
                    triProjected.t[2].y *= 0.5f * (float)ScreenDimensions.y;

                    // Store triangle for sorting
                    vecTrianglesToRaster.push_back(triProjected);
                }
            }
        }

        //Sort triangles from back to front
        std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](Triangle& t1, Triangle& t2)
            {
                float z1 = (t1.t[0].z + t1.t[1].z + t1.t[2].z) / 3.0f;
                float z2 = (t2.t[0].z + t2.t[1].z + t2.t[2].z) / 3.0f;
                return z1 > z2;
            });        



        // Draw Triangles Projections
        for (auto& triProjected : vecTrianglesToRaster)
        {
            sf::VertexArray triVertex(sf::Triangles, 3);
            sf::VertexArray lineVertex(sf::Lines, 6); // 3 lines for the triangle

            objMgr.handleTriangles(triVertex, triProjected);
            window.draw(triVertex);

            if (drawWireFrames)
            {
                objMgr.handleWireFrame(lineVertex, triProjected);
                window.draw(lineVertex);
            }
                
        }

        window.display();
    }
    return 0;
}


// Function to get shading color based on dot product
sf::Color getColor(float& dp)
{
    // Example: Darker color for surfaces facing away from the light
    int shadeValue = static_cast<int>(255.0f * std::max(0.0f, dp));
    return sf::Color(shadeValue, shadeValue, shadeValue);
}






// I couldn't figure this out but you could also draw like this
//for (int p = 0; p < 4; p++)
//{
//    int nTrisToAdd = 0;
//    while (nNewTriangles > 0)
//    {
//        // Take triangle from front of queue
//        Triangle test = listTriangles.front();
//        listTriangles.pop_front();
//        nNewTriangles--;
//
//        // Clip it against a plane. We only need to test each 
//        // subsequent plane, against subsequent new triangles
//        // as all triangles after a plane clip are guaranteed
//        // to lie on the inside of the plane. I like how this
//        // comment is almost completely and utterly justified
//        switch (p)
//        {
//        case 0:	nTrisToAdd = math.Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
//        case 1:	nTrisToAdd = math.Triangle_ClipAgainstPlane({ 0.0f, (float)ScreenDimensions.y - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
//        case 2:	nTrisToAdd = math.Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
//        case 3:	nTrisToAdd = math.Triangle_ClipAgainstPlane({ (float)ScreenDimensions.x - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
//        }
//
//        // Clipping may yield a variable number of triangles, so
//        // add these new ones to the back of the queue for subsequent
//        // clipping against next planes
//        for (int w = 0; w < nTrisToAdd; w++)
//            listTriangles.push_back(clipped[w]);
//    }
//    nNewTriangles = listTriangles.size();
//}
//
//// Draw Triangles
//for (auto& triProjected : listTriangles)
//{
//    sf::VertexArray vertexArray(sf::Triangles, 3);
//
//    for (int i = 0; i < 3; i++)
//    {
//        vertexArray[i].position = sf::Vector2f(triProjected.t[i].x, triProjected.t[i].y);
//        if (i % 2 == 0)
//            vertexArray[i].color = sf::Color::White;
//        else
//            vertexArray[i].color = greyishColor;
//    }
//
//    window.draw(vertexArray);
//}
//
//window.display();
//        }