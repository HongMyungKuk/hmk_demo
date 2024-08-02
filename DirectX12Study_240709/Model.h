#pragma once

#include "AnimationData.h"
#include "ConstantBuffer.h"
#include "Mesh.h"

using namespace DirectX;

class Model
{
  public:
    Model();
    virtual ~Model();

  public:
    void Initialize(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, std::vector<MeshData> meshes,
                    std::vector<MaterialConsts> materials = {}); // const buffer, mesh data
    virtual void Update();
    virtual void Render(ID3D12GraphicsCommandList *commandList);
    virtual void RenderNormal(ID3D12GraphicsCommandList *commandList);
    void UpdateWorldMatrix(Matrix worldRow);

  private:
    virtual void BuildMeshBuffers(ID3D12Device *device, Mesh &mesh, MeshData &meshData);
    void BuildTexture(ID3D12Device *device, ID3D12GraphicsCommandList *commandList, const std::string &filename,
                      ID3D12Resource **texture, ID3D12Resource **uploadTexture, DescriptorHandle &handle,
                      bool isSRGB = false);
    void DestroyMeshBuffers();
    void DestroyTextureResource();

  public:
    virtual ID3D12PipelineState *GetPSO(bool isWireFrame)
    {
        return isWireFrame ? Graphics::defaultWirePSO : Graphics::defaultSolidPSO;
    }

    virtual ID3D12PipelineState *GetDepthOnlyPSO()
    {
        return Graphics::depthOnlyPSO;
    }

    MeshConsts &GetMeshConstCPU()
    {
        return m_meshConstsData;
    }
    MaterialConsts &GetMaterialConstCPU()
    {
        return m_materialConstData;
    }
    Matrix &GetWorldRow()
    {
        return m_world;
    }

    enum MOVE_TYPE
    {
        FRONT = 0,
        SIDE  = 1,
        BACK  = 2,

        END,
    };

    const float GetSpeed(MOVE_TYPE type)
    {
        return m_speed[type];
    }

    void MoveFront(const float dt)
    {
        m_world *= Matrix::CreateTranslation(m_speed[FRONT] * Vector3(0.0f, 0.0f, -1.0f) * dt);
        m_pos = Vector3::Transform(m_pos, Matrix::CreateTranslation(m_speed[FRONT] * Vector3(0.0f, 0.0f, -1.0f) * dt));
        UpdateWorldMatrix(m_world);

        // light의 움직임도 등록.
    }
    void MoveBack(const float dt)
    {
        m_world *= Matrix::CreateTranslation(m_speed[BACK] * Vector3(0.0f, 0.0f, -1.0f) * -dt);
        m_pos = Vector3::Transform(m_pos, Matrix::CreateTranslation(m_speed[BACK] * Vector3(0.0f, 0.0f, -1.0f) * -dt));
        UpdateWorldMatrix(m_world);
    }
    void MoveRight(const float dt)
    {
        m_world *= Matrix::CreateTranslation(m_speed[SIDE] * Vector3(-1.0f, 0.0f, 0.0f) * dt);
        m_pos = Vector3::Transform(m_pos, Matrix::CreateTranslation(m_speed[SIDE] * Vector3(-1.0f, 0.0f, 0.0f) * dt));
        UpdateWorldMatrix(m_world);
    }
    void MoveLeft(const float dt)
    {
        m_world *= Matrix::CreateTranslation(m_speed[SIDE] * Vector3(1.0f, 0.0f, 0.0f) * dt);
        m_pos = Vector3::Transform(m_pos, Matrix::CreateTranslation(m_speed[SIDE] * Vector3(1.0f, 0.0f, 0.0f) * dt));
        UpdateWorldMatrix(m_world);
    }

    void Move(const float dt)
    {
        m_world *= Matrix::CreateTranslation(m_speed[0] * m_velocity * dt);
        m_pos = Vector3::Transform(m_pos, Matrix::CreateTranslation(m_speed[SIDE] * m_velocity * dt));
        UpdateWorldMatrix(m_world);
    }

    void SetSpeed(float s, int type)
    {
        m_speed[type] = s;
    }

    const Vector3& GetPos()
    {
        return m_pos;
    }

    void SetPos(const Vector3& pos)
    {
        m_pos = pos;
    }

    void SetVelocity(const Vector3 v)
    {
        m_velocity = v;
    }

    void AddVelocity(const Vector3 v)
    {
        m_velocity += v;
        // m_velocity.Normalize();
    }

    Vector3 GetVelocity()
    {
        return m_velocity;
    }

  private:
    ID3D12RootSignature *m_rootSignature   = nullptr;
    ID3D12PipelineState *m_pipelineState   = nullptr;
    std::vector<Mesh> m_meshes             = {};
    std::vector<MaterialConsts> m_material = {};
    uint32_t m_descRef                     = 0;
    uint32_t m_descNum                     = 300;

    UploadBuffer<MeshConsts> m_meshUpload;
    UploadBuffer<MaterialConsts> m_materialUpload;
    MeshConsts m_meshConstsData        = {};
    MaterialConsts m_materialConstData = {};

    BoundingSphere m_boundingSphere = {};

    Matrix m_world   = Matrix();
    Matrix m_worldIT = Matrix();

    Vector3 m_pos      = Vector3(0.0f);
    float m_speed[END] = {0.0005f, 0.0005f, 0.00025f};

    uint32_t m_cbvDescriptorSize = 0;

    DescriptorHandle m_handle;
    uint32_t m_texHandleIdx = 0;

    Vector3 m_velocity = Vector3(0.0f);
};
