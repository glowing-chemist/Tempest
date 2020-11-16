#include "Engine/Engine.hpp"
#include "Engine/StaticMesh.h"

#include "GLFW/glfw3.h"

#include <glm/gtx/transform.hpp>

#include "Controller.hpp"
#include "Player.hpp"
#include "RenderThread.hpp"
#include "ScriptEngine.hpp"

void setupGraphicsState(Engine* eng)
{
    eng->registerPass(PassType::DepthPre);
    eng->registerPass(PassType::Shadow);
    eng->registerPass(PassType::GBufferPreDepth);
    eng->registerPass(PassType::DeferredPBRIBL);
    eng->registerPass(PassType::DFGGeneration);
    eng->registerPass(PassType::Skybox);
    eng->registerPass(PassType::ConvolveSkybox);
    eng->registerPass(PassType::Composite);
    eng->registerPass(PassType::Animation);
    eng->registerPass(PassType::LineariseDepth);
#ifndef NDEBUG
    eng->registerPass(PassType::DebugAABB);
#endif
}


int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // only resize explicitly
    auto* window = glfwCreateWindow(1920, 1080, "Tempest", nullptr, nullptr);

    Engine* eng = new Engine(window);
    eng->setShadowMapResolution({1024.0f, 1024.0f});

    eng->startFrame(std::chrono::microseconds(0));
    bool firstFrame = true;
    bool shouldClose = false;

    StaticMesh* firstMesh = new StaticMesh(
        "Assets//Meshes//Player2.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals |
        VertexAttributes::Tangents |
        VertexAttributes::Albedo
    );

    StaticMesh* floor = new StaticMesh(
        "Assets//Meshes//plane.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals |
        VertexAttributes::Tangents |
        VertexAttributes::Albedo
    );

    StaticMesh* cube = new StaticMesh(
        "Assets//Meshes//cube.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals |
        VertexAttributes::Tangents |
        VertexAttributes::Albedo
    );

    Controller* controller1 = new Controller(GLFW_JOYSTICK_1);

    Scene testScene("Assets//Materials");
    testScene.setOctreeMaxDivisions(1);

    std::array<std::string, 6> skybox{ "./Assets/Textures/bluecloud_ft.jpg",
                                       "./Assets/Textures/bluecloud_bk.jpg",
                                       "./Assets/Textures/bluecloud_up.jpg",
                                       "./Assets/Textures/bluecloud_dn.jpg",
                                       "./Assets/Textures/bluecloud_rt.jpg",
                                       "./Assets/Textures/bluecloud_lf.jpg" };
    testScene.loadSkybox(skybox, eng);
    Camera shadowCam(float3(-300.0f, 100.0f, -300.0f), glm::normalize(float3(1.0f, -1.0f, 1.0f)), 1920.0f / 1080.0f, 1.0f, 500.0f, 90.0f, CameraMode::Orthographic);
    shadowCam.setOrthographicSize(float2{1024, 1024});
    testScene.setShadowingLight(shadowCam);

    const SceneID player1MeshID = testScene.addMesh(*firstMesh, MeshType::Dynamic);
    const SceneID planeID = testScene.addMesh(*floor, MeshType::Static);
    const SceneID cubeID = testScene.addMesh(*cube, MeshType::Static);

    const InstanceID player1Instance = testScene.addMeshInstance(player1MeshID, kInvalidInstanceID, float4x4(1.0f), 0, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals | MaterialType::AmbientOcclusion);
    for(int32_t x = -1000; x < 1000; x += 150)
    {
        for(int32_t y = -1000; y < 1000; y += 150)
        {
            testScene.addMeshInstance(cubeID, kInvalidInstanceID, glm::translate(float3{x, 50.0f, y}) * glm::scale(float3(30.0f, 100.0f, 30.0f)), 5, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals, "Pillar");
        }
    }
    const InstanceID groundInstance =  testScene.addMeshInstance(planeID, kInvalidInstanceID, glm::scale(float3(1000.0f, 1000.0f, 1000.0f)) *  glm::rotate(glm::radians(-90.0f), float3(1.0f, 0.0f, 0.0f)), 5, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals);

    RayTracingScene rtScene(eng, &testScene);

    testScene.loadMaterials(eng);
    testScene.uploadData(eng);

    eng->setScene(&testScene);

    eng->getScene()->computeBounds(MeshType::Dynamic);
    eng->getScene()->computeBounds(MeshType::Static);

    setupGraphicsState(eng);

    Camera& camera = eng->getCurrentSceneCamera();
    camera.setFarPlane(200.0f);

    ScriptEngine scriptEngine(eng, &testScene);

    Player* player1 = new Player(player1Instance, testScene.getMeshInstance(player1Instance), float3(0.0f, 0.0f, -60.0f), float3(0.0f, 0.0f, -1.0f));
    player1->attachCamera(camera, 120.0f);
    player1->attachShadowCamera(testScene.getShadowLightCamera());

    size_t frameCount = 0;

    Tempest::RenderThread renderThread(eng);
    auto frameStartTime = std::chrono::system_clock::now();

    while (!shouldClose)
    {
        glfwPollEvents();

        shouldClose = glfwWindowShouldClose(window);

        controller1->update(window);

        {
            std::unique_lock lock = renderThread.lock();

            firstFrame = frameCount == 0;

            renderThread.update(shouldClose, firstFrame);

            const auto currentTime = std::chrono::system_clock::now();
            std::chrono::microseconds frameDelta = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - frameStartTime);
            frameStartTime = currentTime;
            scriptEngine.tick(frameDelta);

            // These update the scene in the engine so need to be guarded with a mutex.
            player1->update(controller1, eng);

            testScene.computeBounds(MeshType::Dynamic);

            std::vector<Scene::Intersection> collisions = testScene.getIntersections(player1Instance);
            for(const auto& collision : collisions)
            {
                if(collision.mEntry2->getName() == "Pillar")
                {
                    player1->undoMove();
                    break;
                }
            }

            renderThread.unlock(lock);
        }

        ++frameCount;
    }

    delete firstMesh;
    delete floor;
    delete controller1;
    delete player1;
}
