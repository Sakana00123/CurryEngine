#pragma once
#include "PropertyDrawContext.h"
#include "Engine/Core/Reflection/Meta.h"
#include <any>

namespace CurryEngine
{
	namespace PropertyDrawHelper
	{
		/**
		 * @brief 複数選択されているオブジェクトのプロパティ値が混在しているかどうかを判定するユーティリティ関数。
		 * @tparam T 判定するプロパティの型。プロパティの getter が返す型と一致させてください。
		 * @param context プロパティ描画のコンテキスト。複数選択されているオブジェクトのリストが含まれます。
		 * @param prop 判定するプロパティのメタ情報。getter を使用してプロパティ値を取得します。
		 * @return 複数選択されているオブジェクトのプロパティ値が混在している場合は true、すべて同じ値の場合は false を返します。
		 */
		template<typename T>
		bool HasMixedValues(const PropertyDrawContext& context, const PropertyInfo& prop)
		{
			if (context.IsEmpty()) return false; // 対象がない場合は混在なしとみなす
			T firstValue = std::any_cast<T>(prop.getter(context.Primary()));
			for (size_t i = 1; i < context.targets.size(); i++)
			{
				if (std::any_cast<T>(prop.getter(context.targets[i])) != firstValue)
				{
					return true; // 型が違うか、値が違う場合は混在しているとみなす
				}
			}
			return false; // すべて同じ値の場合は混在なし
		}
		
		/**
		 * @brief 複数選択されているオブジェクトのプロパティ値を一括で設定するユーティリティ関数。
		 * @tparam T 設定するプロパティの型。プロパティの setter が受け取る型と一致させてください。
		 * @param context プロパティ描画のコンテキスト。複数選択されているオブジェクトのリストが含まれます。
		 * @param prop 設定するプロパティのメタ情報。setter を使用してプロパティ値を設定します。
		 * @param value 設定する値。すべての対象オブジェクトにこの値が設定されます。
		 */
		template<typename T>
		void ApplyToAll(const PropertyDrawContext& context, const PropertyInfo& prop, const T& value)
		{
			for (Component* target : context.targets)
			{
				prop.setter(target, value);
			}
		}

	}
}