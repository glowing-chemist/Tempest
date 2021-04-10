#ifndef PHYSICS_DEBUG_RENDERER_HPP
#define PHYSICS_DEBUG_RENDERER_HPP

#include <LinearMath/btIDebugDraw.h>

class RenderEngine;

namespace Tempest
{
    class PhysicsWorldDebugRenderer : public btIDebugDraw
    {
    public:
        PhysicsWorldDebugRenderer(RenderEngine* eng) :
            mRenderEngine(eng) {}

        ~PhysicsWorldDebugRenderer() = default;

        virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override final;

        virtual void reportErrorWarning(const char* warningString);

        virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {}

        virtual void draw3dText(const btVector3& location, const char* textString) {}

        virtual void setDebugMode(int debugMode)
        {
            mDebugMode = debugMode;
        }

        virtual int getDebugMode() const
        {
            return mDebugMode;
        }

    private:

        RenderEngine* mRenderEngine;
        int mDebugMode;
    };
}

#endif