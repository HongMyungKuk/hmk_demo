#include "pch.h"

#include "Frustum.h"

void Frustum::ConstructFrustum(float screenDepth, Matrix viewMatrix, Matrix projectionMatrix)
{
    Matrix matrix = viewMatrix * projectionMatrix;

    float x = (float)(matrix._14 + matrix._13);
    float y = (float)(matrix._24 + matrix._23);
    float z = (float)(matrix._34 + matrix._33);
    float w = (float)(matrix._44 + matrix._43);

    m_plane[0] = XMPlaneNormalize(Vector4(x, y, z, w));

    x = (float)(matrix._14 - matrix._13);
    y = (float)(matrix._24 - matrix._23);
    z = (float)(matrix._34 - matrix._33);
    w = (float)(matrix._44 - matrix._43);

    m_plane[1] = XMPlaneNormalize(Vector4(x, y, z, w));

    x = (float)(matrix._14 + matrix._11);
    y = (float)(matrix._24 + matrix._21);
    z = (float)(matrix._34 + matrix._31);
    w = (float)(matrix._44 + matrix._41);

    m_plane[2] = XMPlaneNormalize(Vector4(x, y, z, w));

    x = (float)(matrix._14 - matrix._11);
    y = (float)(matrix._24 - matrix._21);
    z = (float)(matrix._34 - matrix._31);
    w = (float)(matrix._44 - matrix._41);

    m_plane[3] = XMPlaneNormalize(Vector4(x, y, z, w));

    x = (float)(matrix._14 + matrix._12);
    y = (float)(matrix._24 + matrix._22);
    z = (float)(matrix._34 + matrix._32);
    w = (float)(matrix._44 + matrix._42);

    m_plane[4] = XMPlaneNormalize(Vector4(x, y, z, w));

    x = (float)(matrix._14 - matrix._12);
    y = (float)(matrix._24 - matrix._22);
    z = (float)(matrix._34 - matrix._32);
    w = (float)(matrix._44 - matrix._42);

    m_plane[5] = XMPlaneNormalize(Vector4(x, y, z, w));
}

bool Frustum::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{

    for (int i = 0; i < 6; i++)
    {
        if (XMVectorGetX(XMPlaneDotCoord(m_plane[i],
                                         Vector4(xCenter - radius, yCenter - radius, zCenter - radius, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(m_plane[i],
                                         Vector4(xCenter + radius, yCenter - radius, zCenter - radius, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(m_plane[i],
                                         Vector4(xCenter - radius, yCenter + radius, zCenter - radius, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(m_plane[i],
                                         Vector4(xCenter + radius, yCenter + radius, zCenter - radius, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(m_plane[i],
                                         Vector4(xCenter - radius, yCenter - radius, zCenter + radius, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(m_plane[i],
                                         Vector4(xCenter + radius, yCenter - radius, zCenter + radius, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(m_plane[i],
                                         Vector4(xCenter - radius, yCenter + radius, zCenter + radius, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(m_plane[i],
                                         Vector4(xCenter + radius, yCenter + radius, zCenter + radius, 1.0f))) >= 0.0f)
            continue;

        return false;
    }

    return true;
}
