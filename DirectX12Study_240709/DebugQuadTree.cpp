#include "pch.h"

#include "DebugQuadTree.h"
#include "Model.h"
#include "GeometryGenerator.h"

DebugQuadTree::~DebugQuadTree()
{
    //for (auto m : m_modelList)
    //{
    //    SAFE_DELETE(m);
    //}
}

void DebugQuadTree::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, Terrain *quadTree, std::vector<Model*>& opaqueLists)
{
    m_device = device;
    m_commandList = commandList;

    CreateCubeMeshs(quadTree->m_rootNode, opaqueLists);
}

void DebugQuadTree::Update()
{
    //for (auto m : m_modelList)
    //{
    //    m->Update();
    //}
}

void DebugQuadTree::Render(ID3D12GraphicsCommandList *commandList)
{
    //for (auto m : m_modelList)
    //{
    //    m->GetPSO(true);
    //    m->Render(commandList);
    //}
}

void DebugQuadTree::CreateCubeMeshs(Terrain::QuadTree* node, std::vector<Model*>& opaqueLists)
{
    int count = 0;
    
    for (int i = 0; i < 4; i++)
    {
        if (node->child[i])
        {
            count++;

            CreateCubeMeshs(node->child[i], opaqueLists);
        }
    }

    if (count != 0)
    {
        return;
    }

    MeshData cube = GeometryGenerator::MakeCube(node->radius * 2.0f, node->radius * 2.0f, node->radius * 2.0f);

    Model *model = new Model;
    model->Initialize(m_device, m_commandList, {cube});
    model->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(node->cx, 0.0f, node->cz)));
    model->GetMaterialConstCPU().useEmissiveMap = false;
    model->GetMaterialConstCPU().emissionFactor = Vector3(0.0f, 1.0f, 0.0f);

    opaqueLists.push_back(model);
}
