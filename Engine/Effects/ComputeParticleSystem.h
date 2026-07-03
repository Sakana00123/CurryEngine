#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

#include <vector>

#include "Engine/Core/Misc.h"
#include "Engine/Resources/Texture.h"
#include "Engine/Resources/Shader.h"

#include <functional>

#include "Engine/Rendering/Pipeline/RenderState.h"

#include <ImGradientHDR.h>

class ComputeParticleSystem
{
public:
	BlendState blendState = BlendState::Additive;
	DepthStencilState depthStencilState = DepthStencilState::NoTestNoWrite;


	//パーティクルスレッド数
	static constexpr UINT NumParticleThread = 1024;

	//パーティクル生成用構造体
	struct EmitParticleData
	{
		DirectX::XMFLOAT4 parameter{ 0,1,-1,-1 };	//x : 描画モード, y : 生存時間, z : 生成遅延時間（CPU側）/ 生存時間記録用（GPU側）, w : グラデーションテクスチャインデックス(0〜MaxGradientSlots-1, -1のときグラデーションなし)

		DirectX::XMFLOAT4 position{ 0,0,0,0 };		// xyz: 生成座標, w: イージングモード
		DirectX::XMFLOAT4 rotation{ 0,0,0,0 };		// xyz: 回転 (オイラー角), w: イージングモード
		DirectX::XMFLOAT4 scale{ 1,1,1,1 };			// xy: 開始スケール, z: イージングモード, w: 空き

		DirectX::XMFLOAT4 endPosition{ 0,0,0,0 };	// xyz: 目標座標 (イージングのターゲット), w: イージング時間 (0のときイージングなし)
		DirectX::XMFLOAT4 endRotation{ 0,0,0,0 };	// xyz: 目標回転 (オイラー角, イージングのターゲット), w: イージング時間 (0のときイージングなし)
		DirectX::XMFLOAT4 endScale{ 1,1,1,1 };		// xy: 目標スケール (イージングのターゲット), z: イージング時間 (0のときイージングなし), w: 空き

		DirectX::XMFLOAT4 velocity{ 0,0,0,0 };		// xyz: 初速, w: 空き
		DirectX::XMFLOAT4 acceleration{ 0,0,0,0 };	//加速度

		float startSpeed = 1.0f;					// 開始時の速度倍率
		float endSpeed = 1.0f;						// 終了時の速度倍率
		float speedEasingMode = 0.0f;				// 速度のイージングモード（0:Linear、1~:各種イージング関数）
		float speedEasingTime = 0.0f;				// 速度のイージング時間（0のときイージングなし）


		DirectX::XMFLOAT4 startColor{ 1,1,1,1 };	// 開始色
		DirectX::XMFLOAT4 endColor{ 1,1,1,1 };		// 終了色

		float fadeInTime = 0.0f;					//フェードイン時間 (4bytes) (0のときフェードインなし)
		float fadeOutTime = 0.0f;					//フェードアウト時間 (4bytes) (0のときフェードアウトなし)
		BYTE padding[8]{};					//パディング（構造体サイズを16の倍数にするため）

	};

	//パーティクル構造体
	//アプリケーション側では使用しないが、形式として必要なのでここで宣言しておく
	struct ParticleData
	{
		DirectX::XMFLOAT4 parameter{ 0,1,-1,-1 };	//x : 描画モード, y : 生存時間, z : 生成遅延時間（CPU側）/ 生存時間記録用（GPU側）, w : グラデーションテクスチャインデックス(0〜MaxGradientSlots-1, -1のときグラデーションなし)

		DirectX::XMFLOAT4 position{ 0,0,0,0 };		// xyz: 座標, w: イージングモード
		DirectX::XMFLOAT4 rotation{ 0,0,0,0 };		// xyz: 回転 (オイラー角), w: イージングモード
		DirectX::XMFLOAT4 scale{ 1,1,1,1 };			// xy : スケール, z: イージングモード, w: 空き

		DirectX::XMFLOAT4 initialPosition{ 0,0,0,0 };	// xyz: 初期座標, w: 空き
		DirectX::XMFLOAT4 initialRotation{ 0,0,0,0 };	// xyz: 初期回転 (オイラー角), w: 空き
		DirectX::XMFLOAT4 initialScale{ 1,1,1,1 };		// xy: 初期スケール, zw: 空き

		DirectX::XMFLOAT4 endPosition{ 0,0,0,0 };	// xyz: 目標座標 (イージングのターゲット), w: イージング時間 (0のときイージングなし)
		DirectX::XMFLOAT4 endRotation{ 0,0,0,0 };	// xyz: 目標回転 (オイラー角, イージングのターゲット), w: イージング時間 (0のときイージングなし)
		DirectX::XMFLOAT4 endScale{ 1,1,1,1 };		// xy: 目標スケール (イージングのターゲット), z: イージング時間 (0のときイージングなし), w: 空き

		DirectX::XMFLOAT4 velocity{ 0,0,0,0 };		// xyz: 初速, w: イージングモード
		DirectX::XMFLOAT4 acceleration{ 0,0,0,0 };	//加速度

		DirectX::XMFLOAT4 initialVelocity{ 0,0,0,0 };		// xyz: 初期速度, w: 空き
		float startSpeed = 1.0f;					// 開始時の速度倍率
		float endSpeed = 1.0f;						// 終了時の速度倍率
		float speedEasingMode = 0.0f;				// 速度のイージングモード（0:Linear、1~:各種イージング関数）
		float speedEasingTime = 0.0f;				// 速度のイージング時間（0のときイージングなし）

		DirectX::XMFLOAT4 texcoord;					//UV情報
		DirectX::XMFLOAT4 color;					//色情報
		DirectX::XMFLOAT4 startColor{ 1,1,1,1 };	// 開始色
		DirectX::XMFLOAT4 endColor{ 1,1,1,1 };		// 終了色

		float fadeInTime = 0.0f;					//フェードイン時間 (4bytes) (0のときフェードインなし)
		float fadeOutTime = 0.0f;					//フェードアウト時間 (4bytes) (0のときフェードアウトなし)
		BYTE padding[8]{};					//パディング（構造体サイズを16の倍数にするため）
	};

	//パーティクルヘッダー構造体
	struct ParticleHeader
	{
		UINT alive;			//生存フラグ
		UINT particleIndex;	//パーティクル番号
		float depth;		//深度
		UINT dummy;
	};

	//汎用情報定義
	struct CommonConstants
	{
		//float deltaTime;					//デルタタイム
		DirectX::XMUINT2 textureSplitCount;	//テクスチャの分割数
		UINT systemNumParticles;			//パーティクル総数
		UINT totalEmitCount;				//現在のフレームでのパーティクル総生成数
		UINT maxEmitParticles;				//現在のフレームでのパーティクル最大生成数
		UINT commonDummy[3];
	};

	//バイトニックソート情報定義
	struct BitonicSortConstants
	{
		UINT increment;
		UINT direction;
		UINT dummy[2];
	};
	static constexpr UINT BitonicSortB2Thread = 256;
	static constexpr UINT BitonicSortC2Thread = 512;

public:
	ComputeParticleSystem(ID3D11Device* device, UINT particlesCount, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView,
		DirectX::XMUINT2 splitCount = DirectX::XMUINT2(1, 1));
	~ComputeParticleSystem();

	void Emit(const EmitParticleData& data);
	void PixelEmitBegin(ID3D11DeviceContext* immediateContext, float deltaTime);
	void PixelEmitEnd(ID3D11DeviceContext* immediateContext);

	void Update(ID3D11DeviceContext* immediateContext, float deltaTime);
	void Render(ID3D11DeviceContext* immediateContext);

	void DrawGUI();

private:
	UINT numParticles;//パーティクル総数
	UINT numEmitParticles;//1フレームで生成可能なパーティクル数
	bool oneShotInitialize;//初期化フラグ
	DirectX::XMUINT2 textureSplitCount;//テクスチャ分割数
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;//パーティクル描画用テクスチャ

	std::vector<EmitParticleData> pendingParticles;//エミット待ちパーティクル
	std::vector<EmitParticleData> emitParticles;//現在のフレームでエミットするパーティクル
	Microsoft::WRL::ComPtr<ID3D11Buffer> commonConstantBuffer;//共通定数バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> bitonicSortConstantBuffer;//バイトニックソート用定数バッファ

	//パーティクルバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleDataShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleDataUnordredAccessView;

	//未使用パーティクル番号を格納したAppend/Cosumeバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleAppendConsumeBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleAppendConsumeUnordredAccessView;

	//パーティクル生成情報を格納したバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleEmitBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleEmitShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleEmitUnordredAccessView;

	//DispatchIndirect用構造体
	using DispatchIndirect = DirectX::XMUINT3; //UINT3で十分

	//	00バイト目：現在のパーティクル総数
	//	04バイト目：1フレーム前のパーティクル総数
	//	08バイト目：パーティクル破棄数
	//	12バイト目：パーティクル生成用DispatchIndirect情報
	static constexpr UINT NumCurrentParticleOffset = 0;
	static constexpr UINT NumPreviousParticleOffset = NumCurrentParticleOffset + sizeof(UINT);
	static constexpr UINT NumDeadParticleOffset = NumPreviousParticleOffset + sizeof(UINT);
	static constexpr UINT EmitDispatchIndirectOffset = NumDeadParticleOffset + sizeof(UINT);
	//DrawInstanced用DrawIndirect用構造体
	struct DrawIndirect
	{
		UINT vertexCountPerInstance;
		UINT instanceCount;
		UINT startVertexLocation;
		UINT startInstanceLocation;
	};
	//	24バイト目：パーティクル更新用DispatchIndirect情報
	//	36バイト目：パーティクル生成時に使用するインデックス(Append/Consumeの代わり)
	//	40バイト目：DrawIndirect情報
	//	40バイト目：ピクセルパーティクル生成数カウンター
	//	44バイト目：DrawIndirect情報
	static constexpr UINT UpdateDispatchIndirectOffset = EmitDispatchIndirectOffset + sizeof(DispatchIndirect);
	static constexpr UINT NumEmitParticleIndexOffset = UpdateDispatchIndirectOffset + sizeof(DispatchIndirect);
	static constexpr UINT NumEmitPixelParticleIndirectOffset = NumEmitParticleIndexOffset + sizeof(UINT);
	static constexpr UINT DrawIndirectOffset = NumEmitPixelParticleIndirectOffset + sizeof(UINT);

	static constexpr UINT DrawIndirectSize = DrawIndirectOffset + sizeof(DrawIndirect);

	//DrawIndirectを用いるため、RWStructuredBufferを用いるものに変更
	Microsoft::WRL::ComPtr<ID3D11Buffer> indirectDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> indirectDataUnordredAccessView;

	//パーティクルヘッダーバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleHeaderBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleHeaderShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particleHeaderUnordredAccessView;

	//各種シェーダー
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> initShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> emitShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> updateShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> beginFrameShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> endFrameShader;

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> sortB2Shader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> sortC2Shader;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;


	// ---- グラデーション関連 ----
public:
	// グラデーションスロット最大数
	static constexpr UINT MaxGradientSlots = 16;

	// 指定スロットにグラデーションをベイクして書き込む
	// slot : エミッタのインデックスに対応（parameter.wに渡す値と一致させる）
	void SetGradient(UINT slot, const ImGradientHDRState& state);

private:
	// グラデーションTexture1DArray本体
	Microsoft::WRL::ComPtr<ID3D11Texture1D> gradientTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> gradientTextureSRV;

	// ベイク解像度
	static constexpr UINT GradientResolution = 128;

	// Texture1DArray 初期化
	void InitGradientTexture(ID3D11Device* device);
	

};