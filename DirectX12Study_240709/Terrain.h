#pragma once

#include "Mesh.h"

class Frustum;
class Model;

class Terrain
{
public:
	virtual ~Terrain()
	{
		Destroy();
	}

private:
	struct QuadTree
	{
		float cx;
		float cz;
		float radius;
		QuadTree* child[4] = {};
		MeshData meshData;
		Model* model = nullptr;
	};

	void InitDivideQuad(QuadTree** node, const float cx, const float cz, const float radius, const MeshData& m, std::vector<Model*>& opaqueLists);
	MeshData GetTriangeInArea(float cx, float cz, float radius, const MeshData& m);
	int32_t GetTriangleCount(float cx, float cz, float radius, const MeshData& m);
	void GetHeight(QuadTree* node, float x, float z, float* height);
	bool IsinsideTriangle(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 n, float x, float z, float* height);
	void UpdateNode(QuadTree* node);
	void RenderNode(QuadTree* node);
	void DestroyNode(QuadTree* node);

public:
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<MeshData> meshData, std::vector<Model*>& opaqueLists);
	void Destroy();

	uint32_t GetMeshComponentSize()
	{
		return m_meshCompCount;
	}
	uint32_t GetRenderTerrainDivideCube()
	{
		return m_meshCompRenderCount;
	}
	void GetObjectHeight(float x, float z, float* height);
	void Render(Frustum* frustum);
	void Update();

protected:
	QuadTree* m_rootNode = nullptr;
	uint32_t m_meshCompCount = 0;
	uint32_t m_meshCompRenderCount = 0;
	Frustum* m_frustum;

	ID3D12Device* m_device = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;

	friend class DebugQuadTree;
};
