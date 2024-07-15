#pragma once

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

struct AnimationClip
{
    struct Key
    {
        Vector3 pos    = Vector3(0.0f);
        Vector3 scale  = Vector3(1.0f);
        Quaternion rot = Quaternion();

        Matrix GetTransform()
        {
            return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rot) * Matrix::CreateTranslation(pos);
        }
    };

    std::string name;
    double duration;
    double tickPerSecond;
    unsigned int numChannels;
    std::vector<std::vector<Key>> keys; // bone의 갯수만큼 존재한다. keys[num bone][num frame]
};

struct AnimationData
{
    std::unordered_map<std::string, int32_t> boneNameToId;
    std::map<uint32_t, std::string> boneIdToName;
    std::vector<int32_t> boneParentId;
    std::vector<Matrix> offsetMatrix;
    std::vector<Matrix> boneTransform;
    Matrix defaultMatrix;
    Matrix accumulrateRootTransform;
    std::vector<AnimationClip> clips;
    Vector3 prevPos = Vector3(0.0f);

    Matrix Get(int clipID, int boneID, int frameCount)
    {
        return defaultMatrix.Invert() * offsetMatrix[boneID] * boneTransform[boneID] * defaultMatrix;
    }

    void Update(int clipID, int frame)
    {
        auto &clip = clips[clipID];

        for (int boneID = 0; boneID < boneTransform.size(); boneID++)
        {
            auto &keys          = clip.keys[boneID];
            const int parentIdx = boneParentId[boneID];
            Matrix parentMatrix = parentIdx >= 0 ? boneTransform[parentIdx] : accumulrateRootTransform;

            auto key = keys.size() > 0 ? keys[frame % keys.size()] : AnimationClip::Key();

            if (parentIdx < 0)
            {
                if (frame != 0)
                {
                    accumulrateRootTransform = Matrix::CreateTranslation(key.pos - prevPos) * accumulrateRootTransform;
                }
                else
                {
                    auto temp = accumulrateRootTransform.Translation();
                    temp.y    = key.pos.y;
                    accumulrateRootTransform.Translation(temp);
                }
                prevPos = key.pos;
                key.pos = Vector3(0.0f);
            }

            boneTransform[boneID] = key.GetTransform() * parentMatrix;
        }
    }
};