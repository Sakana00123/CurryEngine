#pragma once

#include "RenderPass.h"

/** @brief OpaquePass を表すクラスです。 */
class OpaquePass : public RenderPass
{
public:

	// OpaquePassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

};

/** @brief PreviewPass を表すクラスです。 */
class PreviewPass : public RenderPass
{
public:
	// PreviewPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;

};

/** @brief AnimationPreviewPass を表すクラスです。 */
class AnimationPreviewPass : public RenderPass
{
public:
	// AnimationPreviewPassの実装
	/**
	 * @brief 処理を実行します。
	 */
	void Execute(RenderContext* rtx, Scene* scene) override;
};
