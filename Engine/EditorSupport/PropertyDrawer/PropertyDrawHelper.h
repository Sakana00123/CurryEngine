#pragma once
#include "Engine/EditorSupport/PropertyDrawContext.h"
#include "Engine/EditorSupport/PropertyDrawer/DrawerState.h"
#include "Engine/Core/Reflection/Meta.h"
#include "Engine/Editor/History.h"
#include "Engine/EditorSupport/SetValueCommand.h"
#include "Engine/Core/Component.h"
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

		/**
		 * @brief プロパティのラベルを描画するユーティリティ関数。プロパティの名前と Tooltip 属性を使用して、ImGui のラベルを描画します。
		 */
		void BeginPropertyLabel(const PropertyInfo& prop);


		// --- コマンド発行ユーティリティ ---
		struct SetValueCommandDesc
		{
			PropertyInfo prop; // 変更するプロパティのメタ情報
			PropertyDrawContext ctx; // プロパティ描画のコンテキスト
		};

		/**
		 * @brief プロパティの編集を確定するユーティリティ関数。プロパティの前回値と現在値を比較して、変更があった場合にコマンドを発行します。
		 * @tparam T 編集するプロパティの型。プロパティの getter/setter が使用する型と一致させてください。
		 * @param prop 編集するプロパティのメタ情報。getter/setter を使用してプロパティ値を取得/設定します。
		 * @param ctx プロパティ描画のコンテキスト。複数選択されているオブジェクトのリストが含まれます。
		 * @param state ドロワーの状態管理オブジェクト。前回値を保存するために使用します。
		 * @param currentValue 現在のプロパティ値。ImGui の編集ウィジェットから取得した値を渡してください。
		 * @param toStr Undoログ用文字列化関数。変更前後の値を文字列化して Undo ログに記録したい場合は、この関数を渡してください。引数はプロパティ値で、戻り値は文字列です。
		 * @param equals 値の等価比較関数。プロパティ値の型によっては、単純な等価比較が適切でない場合があります（例: 浮動小数点数やクォータニオン）。その場合は、この関数を渡して、値の等価性を適切に判断してください。引数は (現在値, 前回値) で、戻り値は bool です。
		 */
		template<typename T>
		void CommitEdit(
			const PropertyInfo& prop,
			const PropertyDrawContext& ctx,
			DrawerState<T>& state,
			const T& currentValue,
			std::function<std::string(const T&)> toStr = nullptr,
			std::function<bool(const T&, const T&)> equals = nullptr
			)
		{
			if (ctx.IsEmpty())
				return;
			if (!prop.setter || !prop.getter)
				return;
			if (ImGui::IsItemActivated())
				state.Prev(prop.name) = currentValue;

			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				T prev = state.Prev(prop.name);
				bool same = equals ? equals(currentValue, prev) : (currentValue == prev);
				if (!same)
				{
					// Undo/Redo コマンド発行
					std::string newStr = toStr ? toStr(currentValue) : "";
					std::string oldStr = toStr ? toStr(prev) : "";
					{
						std::string description = "Set " + prop.name;
						SetValueCommandDesc descriptionStruct{ prop, ctx };

						CurryEngine::History::ExecuteCommand(
							std::make_shared<SetValueCommand<std::pair<SetValueCommandDesc, T>>>(
								description,
								[](const std::pair<SetValueCommandDesc, T>& data) {
									// Execute: 新しい値をセット
									const auto& [desc, newValue] = data;
									PropertyDrawHelper::ApplyToAll<T>(desc.ctx, desc.prop, newValue);
								},
								std::make_pair(descriptionStruct, prev), // oldValue
								std::make_pair(descriptionStruct, currentValue) // newValue
							)
						);
					}
				}
				state.Prev(prop.name) = currentValue;
			}
		}

	}
}