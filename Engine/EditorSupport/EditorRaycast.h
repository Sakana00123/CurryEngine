#pragma once
#include "Engine/Core/Math/Vector3.h"
#include <memory>
class GameObject;
class Scene;

namespace CurryEngine
{
	namespace EditorSupport
	{
		struct EditorRaycastResult
		{
			bool hit;           //!< レイが何かに当たったか
			float distance;     //!< ヒットした場合の距離（当たらなかった場合は未定義）
			std::weak_ptr<GameObject> hitObject; //!< ヒットしたゲームオブジェクトへのポインタ（当たらなかった場合は nullptr）
		};

		/**
		 * @brief レイとAABBの交差判定を行います。
		 * @param origin レイの原点（ワールド空間）。
		 * @param direction レイの方向（ワールド空間、正規化されていることが期待される）。
		 * @param aabbMin AABBの最小点（ワールド空間）。
		 * @param aabbMax AABBの最大点（ワールド空間）。
		 * @param outDistance レイがヒットした場合の距離を出力します。ヒットしなかった場合は変更されません。
		 * @return レイが何かに当たった場合は true、そうでない場合は false。
		 */
		bool RayVsAABB(const Vector3& origin, const Vector3& direction, const Vector3& aabbMin, const Vector3& aabbMax, float& outDistance);

		/**
		 * @brief レイとシーン内のオブジェクトのAABBとの交差判定を行います。
		 * @param origin レイの原点（ワールド空間）。
		 * @param direction レイの方向（ワールド空間、正規化されていることが期待される）。
		 * @param maxDistance レイの最大距離。これを超えるとヒットしないとみなされます。
		 * @param scene 判定対象のシーン。
		 * @param outResult ヒットした場合の結果を出力します。ヒットしなかった場合は変更されません。
		 * @return レイが何かに当たった場合は true、そうでない場合は false。
		 */
		bool RaycastAABBFallback(const Vector3& origin, const Vector3& direction, float maxDistance, Scene* scene, EditorRaycastResult& outResult);
	}
}