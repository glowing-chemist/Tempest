#include "Engine/Engine.hpp"
#include "Engine/StaticMesh.h"

#include "GLFW/glfw3.h"

#include <glm/gtx/transform.hpp>

#include "Controller.hpp"
#include "Player.hpp"

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // only resize explicitly
    auto* window = glfwCreateWindow(1920, 1080, "Tempest", nullptr, nullptr);

    Engine eng(window);

    eng.startFrame();
    bool firstFrame = true;

    StaticMesh* firstMesh = new StaticMesh(
        "Assets//Meshes//Player.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals
    );

    StaticMesh* floor = new StaticMesh(
        "Assets//Meshes//plane.fbx",
        VertexAttributes::Position4 |
        VertexAttributes::TextureCoordinates |
        VertexAttributes::Normals
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
    testScene.loadSkybox(skybox, &eng);
    testScene.setShadowingLight(float3(-150.0f, 100.0f, -150.0f), float3(0.0f, -0.5f, 1.0f), float3(0.0f, -1.0f, 0.0f));

    const SceneID player1MeshID = testScene.addMesh(*firstMesh, MeshType::Dynamic);
    const SceneID player2MeshID = testScene.addMesh(*firstMesh, MeshType::Dynamic);
    const SceneID planeID = testScene.addMesh(*floor, MeshType::Static);

    const InstanceID player1Instance = testScene.addMeshInstance(player1MeshID, float4x4(1.0f), 0, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals | MaterialType::AmbientOcclusion);
    const InstanceID player2Instance = testScene.addMeshInstance(player2MeshID, float4x4(1.0f), 0, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals | MaterialType::AmbientOcclusion);
    const InstanceID groundInstance = testScene.addMeshInstance(planeID, glm::scale(float3(100.0f, 100.0f, 100.0f)) *  glm::rotate(glm::radians(-90.0f), float3(1.0f, 0.0f, 0.0f)), 5, MaterialType::Albedo | MaterialType::Metalness | MaterialType::Roughness | MaterialType::Normals);

    testScene.loadMaterials(&eng);
    testScene.uploadData(&eng);

    eng.setScene(&testScene);

    eng.getScene()->computeBounds(MeshType::Dynamic);
    eng.getScene()->computeBounds(MeshType::Static);

    eng.registerPass(PassType::Shadow);
    eng.registerPass(PassType::GBuffer);
    eng.registerPass(PassType::DeferredPBRIBL);
    eng.registerPass(PassType::DFGGeneration);
    eng.registerPass(PassType::Skybox);
    eng.registerPass(PassType::ConvolveSkybox);
    eng.registerPass(PassType::Composite);
    eng.registerPass(PassType::Animation);

#ifndef NDEBUG
    eng.registerPass(PassType::DebugAABB);
#endif

    Camera& camera = eng.getCurrentSceneCamera();
    camera.setPosition({150.0f, 50.0f, -10.0f});
    camera.setDirection({-1.0f, 0.0f, 0.0f});

    Player* player1 = new Player(player1Instance, testScene.getMeshInstance(player1Instance));
    Player* player2 = new Player(player2Instance, testScene.getMeshInstance(player2Instance));

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        controller1->update();
        controller2->update();

        player1->update(controller1, &eng);
        player2->update(controller2, &eng);

        //camera.moveLeft(-controller->getLeftAxisX());
        //camera.moveForward(-controller->getLeftAxisY());
        //camera.rotatePitch(-controller->getRighAxisY());
        //camera.rotateYaw(-controller->getRighAxisX());

        if (!firstFrame)
            eng.startFrame();

        firstFrame = false;

        eng.getScene()->computeBounds(MeshType::Dynamic);

        eng.recordScene();
        eng.render();
        eng.swap();
        eng.endFrame();
    }

    delete firstMesh;
    delete floor;
    delete controller1;
    delete player1;
    delete controller2;
    delete player2;
}
