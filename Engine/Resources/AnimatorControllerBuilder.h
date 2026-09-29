#pragma once
#include "ModelAsset.h"
#include "AnimatorController.h"

namespace CurryEngine::Resources
{
	struct AnimatorControllerBuildOptions
	{
		bool generateStateForEachAnimationClip = false; // アニメーションクリップごとにステートを生成するかどうか

	};

	class AnimatorControllerBuilder
	{
	public:
		/**
		 * @brief ModelAssetからAnimatorControllerを構築します。
		 * @param modelAsset モデルアセット
		 * @param options ビルドオプション
		 * @return 構築されたAnimatorControllerの共有ポインタ
		 */
		static std::shared_ptr<AnimatorController> BuildFromModelAsset(const ModelAsset& modelAsset, const AnimatorControllerBuildOptions& options = AnimatorControllerBuildOptions());
	};
}
