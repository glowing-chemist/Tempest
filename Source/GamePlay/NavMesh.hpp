#ifndef NAVMESH_HPP
#define NAVMESH_HPP

#include "Engine/GeomUtils.h"

#include <vector>

class RayTracingScene;

class NavMesh
{
public:

    NavMesh(RayTracingScene*, const float2& density);
    ~NavMesh();


private:

    std::vector<float> mHeightMap;

};

#endif
