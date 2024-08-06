#include "pch.h"

#include "DebugQuadTree.h"
#include "Model.h"
#include "GeometryGenerator.h"

void DebugQuadTree::Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, QuadTree *quadTree)
{
    CreateCubeMeshs(device, commandList, quadTree->GetRootNode());
}

void DebugQuadTree::Update()
{
    for (auto m : m_modelList)
    {
        m->Update();
    }
}

void DebugQuadTree::Render(ID3D12GraphicsCommandList *commandList)
{
    for (auto m : m_modelList)
    {
        m->GetPSO(true);
        m->Render(commandList);
    }
}

void DebugQuadTree::CreateCubeMeshs(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, QuadTree::NodeType *node)
{
    int count = 0;
    
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
        {
            count++;

            CreateCubeMeshs(device, commandList, node->nodes[i]);
        }
    }

    if (count != 0)
    {
        return;
    }

    MeshData cube = GeometryGenerator::MakeCube(node->width, 1.0f, node->width);
    

    Model *model = new Model;
    model->Initialize(device, commandList, {cube});
    model->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(node->positionX, 0.0f, node->positionZ)));
    model->GetMaterialConstCPU().useEmissiveMap = false;
    model->GetMaterialConstCPU().emissionFactor = Vector3(0.0f, 1.0f, 0.0f);

    m_modelList.push_back(model);
}
