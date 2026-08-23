#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Rendering/Renderers/Renderer.h"
#include "Engine/Core/Math/BoundingBox.h"
//#include "Engine/Rendering/Renderers/SkinningData.h"
#include "Engine/Editor/Timeline.h"

#define NOMINMAX

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "Engine/Audio/BeatManager.h"

#include "Engine/Resources/ModelAsset.h"
#include <Engine\Resources\AnimationClip.h>

struct AnimationEvent
{
    struct Event
    {
        float time = 0.0f; // イベント発生時間
        std::function<void()> func; // イベント関数
        bool isCalled = false; // イベントが呼び出されたか(内部用。設定不要)
    };

    std::vector<Event> events; // イベントリスト
};

class GltfModelRenderer : public Renderer
{
	C_REFLECT(GltfModelRenderer)

	C_PROPERTY()
    std::string filePath;

	C_PROPERTY()
    bool animationEnable = true;
	C_PROPERTY()
    bool blendEnable = true;

    bool isBlendStart = false;
    bool isAnimationCompleted = false;
	C_PROPERTY()
    bool enableShadow = true;
    bool onlyShadow = false;

    std::function<void(RenderContext*)> preRenderFunc;
    std::function<void(RenderContext*)> postRenderFunc;

    // アニメーション中のイベント設定
    AnimationEvent animationEvent;

#ifdef _DEBUG
    bool editorStaticBatchingFlag = false;
#endif // _DEBUG

public:
    //Math::BoundingBox boundingBox;
    Math::BoundingBox CalculateAABB() const override;
public:
    void SetPreRenderFunction(const std::function<void(RenderContext*)>& func) {
        preRenderFunc = func;
    }
    void SetPostRenderFunction(const std::function<void(RenderContext*)>& func) {
        postRenderFunc = func;
    }

    void SetEnableShadow(bool enable) {
        enableShadow = enable;
    }
    bool IsEnableShadow() const {
        return enableShadow;
    }

    void SetEnableOnlyShadow(bool enable) {
        onlyShadow = enable;
    }
    bool IsEnableOnlyShadow() const {
        return onlyShadow;
    }

    // ピクセルシェーダーの差し替え
    void ReplacePixelShader(ID3D11Device* device, const char* filePath);
    void ReplaceVertexShader(ID3D11Device* device, const char* filePath);
    void ReplaceCSMVertexShader(ID3D11Device* device, const char* filePath);

    // アニメーション再生
    void SetAnimation(int index, bool blend = true, const AnimationEvent& animEvent = {}) {
        animationIndex = index; // アニメーションインデックスを設定
        time = 0; // アニメーション時間をリセット
        timeRate = 1.0f; // 再生速度をリセット
        isBlendStart = blend; // ブレンド開始フラグを設定
        isAnimationCompleted = false; // アニメーション完了フラグをリセット
        animationEvent = animEvent; // アニメーションイベントを設定
		time = BeatManager::GetTimeInCurrentBeat(); // ビートに同期させる
    }
    // アニメーション再生
    void SetAnimation(const std::string& name, bool blend = true, const AnimationEvent& animEvent = {})
    {
        SetAnimation(GetAnimationIndex(name), blend, animEvent);
    }

	// アニメーション再生
	C_FUNCTION()
    void PlayAnimation(int index, bool blend = true)
    {
		SetAnimation(index, blend);
    }

    // アニメーションの再生速度を設定
    void SetAnimationTimeRate(float rate) { timeRate = rate; }
    // アニメーションの再生速度を取得
    float GetAnimationTimeRate() const { return timeRate; }

    // アニメーションのブレンド時間を設定
    void SetAnimationBlendTime(float blendTime) { animationBlendTime = blendTime; }
    // アニメーションのブレンド時間を取得
    float GetAnimationBlendTime() const { return animationBlendTime; }

    // アニメーションのインデックスを名前から取得
    int GetAnimationIndex(const std::string& name) const {
		auto& animations = m_asset->animations;
        for (int i = 0; i < animations.size(); i++) {
            if (animations[i].name == name) {
                return i;
            }
        }
        return -1;
    }

    void SetStartAnimationTimer(float time)
    {
        this->time = time;
    }

    // 現在のアニメーション名を取得
    std::string GetCurrentAnimationName() const
    {
		auto& animations = m_asset->animations;
        if (animations.size() == 0) return "";
        if (animationIndex < 0 || animationIndex >= animations.size()) return "";
        return animations[animationIndex].name;
    }

    // アニメーションの総数を取得
    int GetAnimationCount() const { return static_cast<int>(m_asset->animations.size()); }

    // 指定したインデックスのアニメーション名を取得
    std::string GetAnimationName(int index) const {
		auto& animations = m_asset->animations;
        if (index < 0 || index >= animations.size()) return "";
        return animations[index].name;
    }

    // アニメーションの長さを取得
    float GetAnimationDuration(int index) const {
		auto& animations = m_asset->animations;
        if (index < 0 || index >= animations.size()) return 0.0f;
        return animations[index].duration;
    }

    // アニメーションの長さを取得
    float GetCurrentAnimationDuration() const {
        return GetAnimationDuration(animationIndex);
    }

    // アニメーションが完了したか
    bool IsAnimationCompleted() const { return isAnimationCompleted; }

    // アニメーションの有効/無効を設定
    void SetAnimationEnable(bool enable) { animationEnable = enable; }

    // アニメーションが有効か
    bool IsAnimationEnable() const { return animationEnable; }

    // ブレンドの有効/無効を設定
    void SetBlendEnable(bool enable) { blendEnable = enable; }
    // ブレンドが有効か
    bool IsBlendEnable() const { return blendEnable; }
    // ループ設定
    void SetLoop(bool loop) { this->loop = loop; }
    // ループ取得
    bool IsLoop() const { return loop; }



	// アニメーションのポーズを適用
    void ApplyPose(const std::vector<NodePose>& poses);

	// ノードの総数を取得
    size_t GetNodeCount() const;

public:
    GltfModelRenderer();
    virtual ~GltfModelRenderer() = default;

	// モデルの読み込み
	void LoadModel(ID3D11Device* device, const std::string& filePath, bool staticBatching);

    void SetModelAsset(std::shared_ptr<ModelAsset> asset);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render(RenderContext* rtx) override;

    void CastShadow(RenderContext* rtx);
#ifdef USE_IMGUI
    void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	// シリアライズ
	json Serialize() const override;

	// デシリアライズ
	void Deserialize(const json& jsonData) override;


	ModelAsset* GetModelAsset() const { return m_asset.get(); }
    

private:

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShaderCsm;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShaderCsm;

    struct PrimitiveConstants {
        DirectX::XMFLOAT4X4 world;
        int material{ -1 };
        int hasTangent{ 0 };
        int skin{ -1 };
        int num{ -1 }; // ?
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveCbuffer;


    static const size_t PRIMITIVE_MAX_JOINTS = 512;
    struct PrimitiveJointConstants {
        DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveJointCbuffer;

    void CreateAndUploadResources(ID3D11Device* device);

	friend class RhythmAnimationController;
	friend class Animator;
	using Node = ModelAsset::Node;
    void Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes);

public:
    float time = 0; // アニメーションの経過時間(秒)
    float timeRate = 1.0f; // アニメーションの再生速度
    float animationBlendTime = 1.2f; // アニメーションのブレンド時間(秒)
    int animationIndex = 0; // 現在のアニメーションインデックス
    bool loop = true;//ループ設定
	std::shared_ptr<ModelAsset> m_asset; // モデルアセット
};
