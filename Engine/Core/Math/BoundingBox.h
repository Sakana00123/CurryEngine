#pragma once
#include "Vector3.h"

#undef min
#undef max

namespace Math
{
	/**
	 * @brief 軸に平行な直方体の境界ボックス。
	 * @details 最小/最大点、中心点、サイズ、包含判定を提供します。
	 */
	struct BoundingBox
	{
		XMFLOAT3 min;    //!< 各軸の最小点
		XMFLOAT3 max;    //!< 各軸の最大点
		/**
		 * @brief 既定コンストラクタ。
		 * @details `min` を非常に大きな値、`max` を非常に小さな値で初期化します。
		 */
		BoundingBox()
			: min(FLT_MAX, FLT_MAX, FLT_MAX)
			, max(-FLT_MAX, -FLT_MAX, -FLT_MAX)
		{}
		/**
		 * @brief 指定した最小/最大点で初期化します。
		 * @param min 各軸の最小点。
		 * @param max 各軸の最大点。
		 */
		BoundingBox(const XMFLOAT3& min, const XMFLOAT3& max)
			: min(min), max(max)
		{}
		/**
		 * @brief 指定した点を含むように拡張します。
		 * @param point 含めたい点。
		 */
		void Encapsulate(const XMFLOAT3& point)
		{
			min.x = (std::min)(min.x, point.x);
			min.y = (std::min)(min.y, point.y);
			min.z = (std::min)(min.z, point.z);
			max.x = (std::max)(max.x, point.x);
			max.y = (std::max)(max.y, point.y);
			max.z = (std::max)(max.z, point.z);
		}
		/**
		 * @brief 指定したボックスを含むように拡張します。
		 * @param box 含めたいボックス。
		 */
		void Encapsulate(const BoundingBox& box)
		{
			min.x = (std::min)(min.x, box.min.x);
			min.y = (std::min)(min.y, box.min.y);
			min.z = (std::min)(min.z, box.min.z);
			max.x = (std::max)(max.x, box.max.x);
			max.y = (std::max)(max.y, box.max.y);
			max.z = (std::max)(max.z, box.max.z);
		}
		/**
		 * @brief ボックスの中心点を取得します。
		 * @return 中心点。
		 */
		Vector3 Center() const
		{
			Vector3 center;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&center), (XMLoadFloat3(&min) + XMLoadFloat3(&max)) * 0.5f);
			return center;
		}
			
		/**
		 * @brief ボックスのサイズを取得します。
		 * @return 各軸のサイズ。
		 */
		Vector3 Size() const
		{
			Vector3 size;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&size), XMLoadFloat3(&max) - XMLoadFloat3(&min));
			return size;
		}
		/**
		 * @brief 指定した点がボックスに含まれるか判定します。
		 * @param point 判定したい点。
		 * @return 含まれる場合 true。
		 */
		bool Contains(const XMFLOAT3& point) const
		{
			return (point.x >= min.x && point.x <= max.x) &&
				(point.y >= min.y && point.y <= max.y) &&
				(point.z >= min.z && point.z <= max.z);
		}

		/**
		 * @brief 指定したボックスがボックスに含まれるか判定します。
		 * @param box 判定したいボックス。
		 * @return 含まれる場合 true。
		 */
		bool Contains(const BoundingBox& box) const
		{
			return (box.min.x >= min.x && box.max.x <= max.x) &&
				(box.min.y >= min.y && box.max.y <= max.y) &&
				(box.min.z >= min.z && box.max.z <= max.z);
		}
	};
}
			