#include "Engine/Engine.hpp"
#include "Engine/StaticMesh.h"

#include "GLFW/glfw3.h"

#include <glm/gtx/transform.hpp>

#include "Controller.hpp"
#include "Player.hpp"
#include "RenderThread.hpp"
#include "ScriptEngine.hpp"


int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // only resize explicitly
    auto* window = glfwCreateWindow(1920, 1080, "Tempest", nullptr, nullptr);

    Engine* eng = new Engine(window);

    eng->startFrame(std::chrono::microseconds(0));
    bool firstFrame = true;
    bool shouldClose = false;

    StaticMesh* firstMesh = new StaticMesh(
        "Assets//Meshes//Player2.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals |
        VertexAttributes::Albedo
    );

    StaticMesh* floor = new StaticMesh(
        "Assets//Meshes//plane.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals |
        VertexAttributes::Albedo
    );

    StaticMesh* cube = new StaticMesh(
        "Assets//Meshes//cube.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals |
        VertexAttributes::Albedo
    );

    Controller* controller1 = new Controller(GLFW_JOYSTICK_1);
    Controller* controller2 = new Controller(GLFW_JOYSTICK_2);

    Scene testScene("Assets//Materials");

    std::array<std::string, 6> skybox{ "./Assets/Textures/bluecloud_ft.jpg",
                                       "./Assets/Textures/bluecloud_bk.jpg",
                                       "./Assets/Textures/bluecloud_up.jpg",
                                       "./Assets/Textures/bluecloud_dn.jpg",
                                       "./Assets/Textures/bluecloud_rt.jpg",
                                       "./Assets/Textures/bluecloud_lf.jpg" };
    testScene.loadSkybox(skybox, eng);
    Camera shadowCam(float3(-150.0f, 200.0f, -150.0f), glm::normalize(float3(0.0f, -0.5f, 1.0f)), 1920.0f / 1080.0f);
    shadowCam.setNearPlane(150.0f);
    shadowCam.setFarPlane(250.0f);
    shadowCam.setFrameBufferSizeOrthographic(float2{200.0f, 200.0f});
    shadowCam.setCameraMode(CameraMode::Orthographic);
    testScene.setShadowingLight(shadowCam);

    const SceneID player1MeshID = testScene.addMesh(*firstMesh, MeshType::Dynamic);
    const SceneID player2MeshID = testScene.addMesh(*firstMesh, MeshType::Dynamic);
    const SceneID planeID = testScene.addMesh(*floor, MeshType::Static);
    const SceneID cubeID = testScene.addMesh(*cube, MeshType::Static);

    const InstanceID player1Instance = testScene.addMeshInstance(player1MeshID, float4x4(1.0f), 0, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals | MaterialType::AmbientOcclusion);
    const InstanceID player2Instance = testScene.addMeshInstance(player2MeshID, float4x4(1.0f), 0, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals | MaterialType::AmbientOcclusion);
    const InstanceID groundInstance =  testScene.addMeshInstance(planeID, glm::scale(float3(100.0f, 100.0f, 100.0f)) *  glm::rotate(glm::radians(-90.0f), float3(1.0f, 0.0f, 0.0f)), 5, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals);
    const InstanceID cubeInstance =    testScene.addMeshInstance(cubeID, glm::scale(float3(30.0f, 100.0f, 30.0f)), 5, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals);

    RayTracingScene rtScene(eng, &testScene);

    testScene.loadMaterials(eng);
    testScene.uploadData(eng);

    eng->setScene(&testScene);

    eng->getScene()->computeBounds(MeshType::Dynamic);
    eng->getScene()->computeBounds(MeshType::Static);

    eng->registerPass(PassType::Shadow);
    eng->registerPass(PassType::GBuffer);
    eng->registerPass(PassType::DeferredPBRIBL);
    eng->registerPass(PassType::DFGGeneration);
    eng->registerPass(PassType::Skybox);
    eng->registerPass(PassType::ConvolveSkybox);
    eng->registerPass(PassType::Composite);
    eng->registerPass(PassType::Animation);
    //eng->registerPass(PassType::LineariseDepth);
    //eng->registerPass(PassType::TAA);
#ifndef NDEBUG
    eng->registerPass(PassType::DebugAABB);
#endif

    Camera& camera = eng->getCurrentSceneCamera();
    camera.setPosition({150.0f, 50.0f, -10.0f});
    camera.setDirection({-1.0f, 0.0f, 0.0f});
    camera.setFarPlane(50.0f);

    ScriptEngine scriptEngine(eng, &testScene);

    Player* player1 = new Player(player1Instance, testScene.getMeshInstance(player1Instance), float3(0.0f, 0.0f, -60.0f), float3(0.0f, 0.0f, -1.0f));
    Player* player2 = new Player(player2Instance, testScene.getMeshInstance(player2Instance), float3(0.0f, 0.0f, 60.0f), float3(0.0f, 0.0f, 1.0f));

    size_t frameCount = 0;

    RenderThread renderThread(eng);
    auto frameStartTime = std::chrono::system_clock::now();

    while (!shouldClose)
    {
        glfwPollEvents();

        shouldClose = glfwWindowShouldClose(window);

        controller1->update(window);
        controller2->update(window);

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
            player2->update(controller2, eng);

            renderThread.unlock(lock);
        }

        {

#if 1 // View detection.

            const float3& player1Pos = player1->getPosition() + float3(0.0f, 10.0f, 0.0f);;
            const float3 player1Dir = glm::normalize(player1->getDirection());

            const float3& player2pos = player2->getPosition() + float3(0.0f, 10.0f, 0.0f);
            const float3& player2Dir = glm::normalize(player2->getDirection());

            const float3 player1ToPlayer2 = glm::normalize(player2pos - player1Pos);
            const float3 pplayer2ToPlayer1 = -player1ToPlayer2;

            const bool visible = rtScene.isVisibleFrom(player1Pos, player2pos);

            if(acos(glm::dot(player1Dir, player1ToPlayer2)) <= 0.78f)
            {
                if(visible)
                    player2->applyForce(player1Dir / 10.0f);
            }

            if(acos(glm::dot(player2Dir, pplayer2ToPlayer1)) <= 0.78f)
            {
                if(visible)
                    player1->applyForce(player2Dir / 10.0f);
            }
#endif

        }

        ++frameCount;
    }

    delete firstMesh;
    delete floor;
    delete controller1;
    delete player1;
    delete controller2;
    delete player2;
}
