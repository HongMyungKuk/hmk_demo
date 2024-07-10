#pragma once

class RootParameter
{
  public:
    void InitAsConstant();

  private:
    D3D12_ROOT_PARAMETER m_rootParam;
};

class RootSignature
{
  public:
    RootSignature()
    {
        Reset(0, 0);
    }

    ~RootSignature()
    {

    }

    void Reset(uint32_t numRootParam, uint32_t numStaticSamplers = 0)
    {
        if (numRootParam > 0)
        {
            m_rootParamArr = new RootParameter[numRootParam];
        }
        else
        {
            m_rootParamArr = nullptr;
        }
        m_numRootParams = numRootParam;

        if (numStaticSamplers > 0)
        {
            m_samplerArr = new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers];
        }
        else
        {
            m_samplerArr = nullptr;
        }
        m_numSamplers = numStaticSamplers;
        m_countSamplers = 0;
    }

    void InitAsStaicSampler(uint32_t regiser, const D3D12_SAMPLER_DESC &nonStaticSamplerDesc,
                            D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

    
    uint32_t m_numRootParams;
    uint32_t m_numSamplers;
    uint32_t m_countSamplers;
    RootParameter *m_rootParamArr = nullptr;
    D3D12_STATIC_SAMPLER_DESC *m_samplerArr = nullptr;
};
