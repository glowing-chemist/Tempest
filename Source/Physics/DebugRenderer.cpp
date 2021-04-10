#include "DebugRenderer.hpp"
#include "Engine/Engine.hpp"

#include "Core/BellLogging.hpp"


namespace Tempest
{
    void PhysicsWorldDebugRenderer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        mRenderEngine->addDebugLine({{from.x(), from.y(), from.z(), 1.0f},
                                     {to.x(), to.y(), to.z(), 1.0f}});
    }


    void PhysicsWorldDebugRenderer::reportErrorWarning(const char* warningString)
    {
#ifndef NDEBUG
        printf("%s\n", warningString);
#endif
    }
}