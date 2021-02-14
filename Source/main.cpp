#include "Engine/Engine.hpp"
#include "Engine/StaticMesh.h"

#include "GLFW/glfw3.h"

#include <glm/gtx/transform.hpp>

#include "Controller.hpp"
#include "Player.hpp"
#include "RenderThread.hpp"
#include "ScriptEngine.hpp"
#include "PhysicsWorld.hpp"


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
    //eng->registerPass(PassType::TAA);
    //eng->registerPass(PassType::SSAO);
//#ifndef NDEBUG
    eng->registerPass(PassType::DebugAABB);
//#endif
}


int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // only resize explicitly
    auto* window = glfwCreateWindow(1920, 1080, "Tempest", nullptr, nullptr);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
        VertexAttributes::Albedo,
        true
    );

    StaticMesh* floor = new StaticMesh(
        "Assets//Meshes//plane.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals |
        VertexAttributes::Tangents |
        VertexAttributes::Albedo,
        true
    );

    StaticMesh* cube = new StaticMesh(
        "Assets//Meshes//cube.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals |
        VertexAttributes::Tangents |
        VertexAttributes::Albedo,
        true
    );

    Controller* controller1 = new Controller(GLFW_JOYSTICK_1);
    Tempest::PhysicsWorld* physicsWorld = new Tempest::PhysicsWorld();

    Scene testScene("Assets//Materials");

    std::array<std::string, 6> skybox{ "./Assets/Textures/bluecloud_ft.jpg",
                                       "./Assets/Textures/bluecloud_bk.jpg",
                                       "./Assets/Textures/bluecloud_up.jpg",
                                       "./Assets/Textures/bluecloud_dn.jpg",
                                       "./Assets/Textures/bluecloud_rt.jpg",
                                       "./Assets/Textures/bluecloud_lf.jpg" };
    testScene.loadSkybox(skybox, eng);
    Camera shadowCam(float3(-1.0f, 0.1f, -1.0f), glm::normalize(float3(1.0f, -1.0f, 1.0f)), 1.0f, 0.1f, 9.0f, 90.0f, CameraMode::Orthographic);
    shadowCam.setOrthographicSize(float2{10, 10});
    testScene.setShadowingLight(shadowCam);

    const SceneID player1MeshID = testScene.addMesh(*firstMesh, MeshType::Dynamic);
    const SceneID planeID = testScene.addMesh(*floor, MeshType::Static);
    const SceneID cubeID = testScene.addMesh(*cube, MeshType::Static);

    const InstanceID player1Instance = testScene.addMeshInstance(player1MeshID,
                                                                 kInvalidInstanceID,
                                                                 glm::translate(float3{0.0f, 5.0f, 0.0f}) * glm::scale(float3{0.05f, 0.05f, 0.05f}),
                                                                 0,
                                                                 MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals | MaterialType::AmbientOcclusion,
                                                                 "Player");
    physicsWorld->addObject(player1Instance,
                            Tempest::PhysicsEntityType::DynamicRigid,
                            Tempest::BasicCollisionGeometry::Capsule,
                            float3{0.0f, 5.0f, 0.0f},
                            float3{0.125f, 0.25f, 0.125f},
                            60.0f);
    // Restrict player capsule rotation
    {
        btRigidBody *playerCapsule = physicsWorld->getRigidBody(player1Instance);
        playerCapsule->setAngularFactor({0.0f, 1.0f, 0.0f});
    }
    std::vector<InstanceID> pillarInstanes{};
    for(float x = -10.0f; x < 10.0f; x += 1.5f)
    {
        for(float y = -10.0f; y < 10.0f; y += 1.5f)
        {
            const InstanceID pillarID = testScene.addMeshInstance(cubeID,
                                                                  kInvalidInstanceID,
                                                                  testScene.getRootTransform() * glm::translate(float3{ x, 0.5f, y }) *
                                                                  glm::scale(float3(30.0f, 100.0f, 30.0f)),
                                                                  5,
                                                                  MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals,
                                                                  "Pillar");
            physicsWorld->addObject(pillarID,
                                    Tempest::PhysicsEntityType::StaticRigid,
                                    Tempest::BasicCollisionGeometry::Box,
                                    float3{ x, 0.0f, y },
                                    float3(0.6f, 3.0f, 0.6f));
            pillarInstanes.push_back(pillarID);
        }
    }
    const InstanceID groundInstance =  testScene.addMeshInstance(planeID,
                                                                 kInvalidInstanceID,
                                                                 testScene.getRootTransform() * glm::rotate(glm::radians(-90.0f),float3(1.0f, 0.0f, 0.0f)) * glm::scale(float3(1000.0f, 1000.0f, 1.0f)),
                                                                 5,
                                                                 MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals,
                                                                 "Ground plane");
    physicsWorld->addObject(groundInstance,
                            Tempest::PhysicsEntityType::StaticRigid,
                            Tempest::BasicCollisionGeometry::Plane,
                            float3{0.0f, 0.0f, 0.0f},
                            float3{});


    testScene.loadMaterials(eng);
    testScene.uploadData(eng);

    eng->setScene(&testScene);

    eng->getScene()->computeBounds(AccelerationStructure::Dynamic);
    eng->getScene()->computeBounds(AccelerationStructure::Static);
    eng->getScene()->computeBounds(AccelerationStructure::Physics);

    setupGraphicsState(eng);

    Camera& camera = eng->getCurrentSceneCamera();
    camera.setFarPlane(100.0f);

    Tempest::ScriptEngine scriptEngine(eng, &testScene);

    Player* player1 = new Player(player1Instance, testScene.getMeshInstance(player1Instance), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, -1.0f));
    player1->attachCamera(camera, 1.2f);
    player1->attachShadowCamera(testScene.getShadowLightCamera());

    Tempest::RenderThread renderThread(eng);
    auto frameStartTime = std::chrono::system_clock::now();

    while (!shouldClose)
    {
        PROFILER_START_FRAME("Start frame");

        glfwPollEvents();

        shouldClose = glfwWindowShouldClose(window);

        const auto currentTime = std::chrono::system_clock::now();
        std::chrono::microseconds frameDelta = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - frameStartTime);
        frameStartTime = currentTime;

        physicsWorld->tick(frameDelta);
        if(physicsWorld->getManifoldCount() > 0)
        {
            int manifoldCount = physicsWorld->getManifoldCount();
            btPersistentManifold** manifolds = physicsWorld->getManifolds();
            for(int i = 0; i < manifoldCount; ++i)
            {
                btPersistentManifold* manifold = manifolds[i];
                const btCollisionObject* obj0 = manifold->getBody0();
                const btCollisionObject* obj1 = manifold->getBody1();

                MeshInstance* inst0 = testScene.getMeshInstance(obj0->getUserIndex());
                MeshInstance* inst1 = testScene.getMeshInstance(obj1->getUserIndex());

                //printf("collision between %s and %s\n", inst0->getName().c_str(), inst1->getName().c_str());
            }
        }
        controller1->update(window);

        if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        {
            {
                btRigidBody *playerBody = physicsWorld->getRigidBody(player1Instance);
                btVector3 playerMin, playerMax;
                playerBody->getAabb(playerMin, playerMax);
                eng->addDebugAABB({float4{playerMin.x(), playerMin.y(), playerMin.z(), 0.0f},
                                   float4{playerMax.x(), playerMax.y(), playerMax.z(), 0.0f}});
            }

            {
                btRigidBody *groundBody = physicsWorld->getRigidBody(groundInstance);
                btVector3 groundMin, groundMax;
                groundBody->getAabb(groundMin, groundMax);
                eng->addDebugAABB({float4{groundMin.x(), groundMin.y(), groundMin.z(), 0.0f},
                                   float4{groundMax.x(), groundMax.y(), groundMax.z(), 0.0f}});
            }

            for (InstanceID id : pillarInstanes) {
                btRigidBody *pillarBody = physicsWorld->getRigidBody(id);
                btVector3 pillarMin, pillarMax;
                pillarBody->getAabb(pillarMin, pillarMax);
                eng->addDebugAABB({float4{pillarMin.x(), pillarMin.y(), pillarMin.z(), 0.0f},
                                   float4{pillarMax.x(), pillarMax.y(), pillarMax.z(), 0.0f}});
            }
        }

        {
            renderThread.update(shouldClose, firstFrame);
            std::unique_lock lock = renderThread.lock();

            scriptEngine.tick(frameDelta);

            player1->update(controller1, eng, physicsWorld);

            renderThread.unlock(lock);
        }

        firstFrame = false;
    }

    delete firstMesh;
    delete floor;
    delete controller1;
    delete player1;
}
