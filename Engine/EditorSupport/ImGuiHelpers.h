#pragma once

#ifdef USE_IMGUI
#include <imgui.h>
#include "Engine/Editor/History.h"
#include "Engine/EditorSupport/SetValueCommand.h"
#include "Engine/Core/Object.h"

// ImGuiの各プロパティの描画を簡略化するためのヘルパー関数群

constexpr float ImGuiPropertyNameWidth = 120.0f; // プロパティ名の幅を固定するための定数

// プロパティセクションの開始
#define IMGUI_PROPERTY_BEGIN() \
		ImGui::BeginTable("PropertyTable", 2, ImGuiTableFlags_SizingFixedFit); \
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, ImGuiPropertyNameWidth); \
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch); \
		
// プロパティセクションの終了
#define IMGUI_PROPERTY_END() \
		ImGui::EndTable();

// プロパティ名の描画(name: 表示名)
#define IMGUI_PROPERTY(name) \
		ImGui::TableNextRow(); \
		ImGui::TableSetColumnIndex(0); \
		ImGui::Text(name); \
		ImGui::TableSetColumnIndex(1);

// プロパティの値の描画。プロパティ名は前の行で IMGUI_PROPERTY マクロを用いて描画されていることが前提。値の描画はこのマクロを呼び出した場所で行う必要がある。
#define IMGUI_PROPERTY_INDENT() \
		ImGui::TableNextRow(); \
		ImGui::TableSetColumnIndex(0); \
		ImGui::Text("  "); /* インデント用の空白を描画 */ \
		ImGui::TableSetColumnIndex(1);

// プロパティ名の描画とツールチップの表示(name: 表示名, tooltip: ツールチップの内容。nullptr の場合はツールチップを表示しない)
#define IMGUI_PROPERTY_EX(name, tooltip) \
		ImGui::TableNextRow(); \
		ImGui::TableSetColumnIndex(0); \
		ImGui::Text(name); \
		if (ImGui::IsItemHovered() && tooltip) { ImGui::SetTooltip("%s", tooltip); } /* ツールチップを表示 */ \
		ImGui::TableSetColumnIndex(1);



// プロパティのUndo・Redoのためのコマンド実行関数マクロ。(name: プロパティ名, type: 型名, newValue: 新しい値, oldValue: 古い値, newValueStr: 新しい値の文字列表現, oldValueStr: 古い値の文字列表現)※Objectクラスを継承しているクラスでのみ使用可能。プロパティのアドレスはObjectクラスのGetPropertyAddress関数を用いて取得される。プロパティが見つからない場合はエラーログが出力される。
#define IMGUI_PROPERTY_COMMAND(name, type, newValue, oldValue, newValueStr, oldValueStr) \
		CurryEngine::History::ExecuteCommand( \
			std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, type>>>( \
				"Set " + std::string(name) + " old:" + oldValueStr + " new:" + newValueStr, \
				[this](const std::pair<std::string, type>& pair) { \
					if (Object* object = dynamic_cast<Object*>(this)) { \
						auto* prop = object->GetClassMeta()->FindProperty(pair.first); /*プロパティのメタ情報を取得*/ \
						if (prop) { \
							prop->setter(this, pair.second); /*プロパティの値を変更*/ \
						} \
						else { \
							LOG_ERROR("Property not found: " + pair.first); /*プロパティが見つからない場合はエラーログを出力*/ \
						} \
					} \
				}, \
				std::make_pair(std::string(name), oldValue), \
				std::make_pair(std::string(name), newValue) \
			) \
		);

// ジェネリックなプロパティコマンド。Objectクラスを継承していなくても使用可能。プロパティの値をセットする関数は引数 setter で渡す必要がある。setter は新しい値を引数に取り、プロパティの値を変更する関数でなければならない。
#define IMGUI_PROPERTY_COMMAND_CUSTOM(name, newValue, oldValue, newValueStr, oldValueStr, setter) \
		CurryEngine::History::ExecuteCommand( \
			std::make_shared<CurryEngine::SetValueCommand<decltype(newValue)>>( \
				"Set " + std::string(name) + " old:" + oldValueStr + " new:" + newValueStr, \
				setter, \
				oldValue, \
				newValue \
			) \
		);

// ジェネリックなプロパティコマンド。Objectクラスを継承していなくても使用可能。値の文字列表現は単純に数値を文字列化したものになる。プロパティの値をセットする関数は引数 setter で渡す必要がある。setter は新しい値を引数に取り、プロパティの値を変更する関数でなければならない。
#define IMGUI_PROPERTY_COMMAND_CUSTOM_SIMPLE(name, newValue, oldValue, setter) \
		IMGUI_PROPERTY_COMMAND_CUSTOM(name, newValue, oldValue, std::to_string(newValue), std::to_string(oldValue), setter)

// ジェネリックなプロパティコマンド。Objectクラスを継承していなくても使用可能。値の文字列表現は単純に数値を文字列化したものになる。デフォルトセッターは単純に変数に値をセットするラムダ関数になる。setter を省略したい場合はこのマクロを使用することができる。
#define IMGUI_PROPERTY_COMMAND_DEFAULT(name, var, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND_CUSTOM_SIMPLE(name, newValue, oldValue, [this](const decltype(newValue)& v) { var = v; }) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \


// 単純な型のプロパティコマンド。値の文字列表現は単純に数値を文字列化したものになる。
#define IMGUI_PROPERTY_COMMAND_SIMPLE(name, type, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND(name, type, newValue, oldValue, std::to_string(newValue), std::to_string(oldValue))

// 文字列型のプロパティコマンド。値の文字列表現はそのまま値になる。
#define IMGUI_PROPERTY_COMMAND_STRING(name, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND(name, std::string, newValue, oldValue, newValue, oldValue)

// 列挙型のプロパティコマンド。値の文字列表現は items 配列から取得される。
#define IMGUI_PROPERTY_COMMAND_ENUM(name, newValue, oldValue, items) \
		IMGUI_PROPERTY_COMMAND(name, int, newValue, oldValue, items[newValue], items[oldValue])

// ブール型のプロパティコマンド。値の文字列表現は "true" または "false" になる。
#define IMGUI_PROPERTY_COMMAND_BOOL(name, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND(name, bool, newValue, oldValue, (newValue ? "true" : "false"), (oldValue ? "true" : "false"))

// 浮動小数点型のプロパティコマンド。値の文字列表現は単純に数値を文字列化したものになる。
#define IMGUI_PROPERTY_COMMAND_FLOAT(name, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND(name, float, newValue, oldValue, std::to_string(newValue), std::to_string(oldValue))

// 整数型のプロパティコマンド。値の文字列表現は単純に数値を文字列化したものになる。
#define IMGUI_PROPERTY_COMMAND_INT(name, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND(name, int, newValue, oldValue, std::to_string(newValue), std::to_string(oldValue))

// Vector2型のプロパティコマンド。値の文字列表現は "(x,y)" の形式で表示される。
#define IMGUI_PROPERTY_COMMAND_VECTOR2(name, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND(name, Vector2, newValue, oldValue, "(" + std::to_string(newValue.x) + "," + std::to_string(newValue.y) + ")", \
			"(" + std::to_string(oldValue.x) + "," + std::to_string(oldValue.y) + ")")

// Vector3型のプロパティコマンド。値の文字列表現は "(x,y,z)" の形式で表示される。
#define IMGUI_PROPERTY_COMMAND_VECTOR3(name, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND(name, Vector3, newValue, oldValue, "(" + std::to_string(newValue.x) + "," + std::to_string(newValue.y) + "," + std::to_string(newValue.z) + ")", \
			"(" + std::to_string(oldValue.x) + "," + std::to_string(oldValue.y) + "," + std::to_string(oldValue.z) + ")")

// Color型のプロパティコマンド。値の文字列表現は "(r,g,b,a)" の形式で表示される。
#define IMGUI_PROPERTY_COMMAND_COLOR(name, newValue, oldValue) \
		IMGUI_PROPERTY_COMMAND(name, Color, newValue, oldValue, \
			"(" + std::to_string(newValue.r) + "," + std::to_string(newValue.g) + "," + std::to_string(newValue.b) + "," + std::to_string(newValue.a) + ")", \
			"(" + std::to_string(oldValue.r) + "," + std::to_string(oldValue.g) + "," + std::to_string(oldValue.b) + "," + std::to_string(oldValue.a) + ")")

// 各型のプロパティ描画関数

// 整数型プロパティ(name: 表示名, var: 変数, edited: 編集されたかどうかを返す変数, ...: ImGui::DragIntの引数)
#define IMGUI_PROPERTY_INT(name, var, edited, ...) \
		{ \
			ImGui::PushID(&var); \
			IMGUI_PROPERTY(name) \
			edited |= ImGui::DragInt("##int", &var, ##__VA_ARGS__); \
			static int prevValue; /* 前回の値を保持する静的変数 */ \
			if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
			{ \
				prevValue = var; \
			} \
			if (ImGui::IsItemDeactivatedAfterEdit()) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
			{ \
				int newValue = var; \
				if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
				{ \
					IMGUI_PROPERTY_COMMAND_DEFAULT(name, var, newValue, prevValue) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
				} \
				prevValue = newValue; /* 前回の値を新しい値に更新 */ \
			} \
			ImGui::PopID(); \
		}

// 浮動小数点型プロパティ(name: 表示名, var: 変数, edited: 編集されたかどうかを返す変数, ...: ImGui::DragFloatの引数)
#define IMGUI_PROPERTY_FLOAT(name, var, edited, ...) \
		 { \
			ImGui::PushID(&var); \
			IMGUI_PROPERTY(name) \
			edited |= ImGui::DragFloat("##float", &var, ##__VA_ARGS__); \
			static float prevValue; /* 前回の値を保持する静的変数 */ \
			if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
			{ \
				prevValue = var; \
			} \
			if (ImGui::IsItemDeactivatedAfterEdit()) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
				{ \
					float newValue = var; \
					if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
					{ \
						IMGUI_PROPERTY_COMMAND_DEFAULT(name, var, newValue, prevValue) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
					} \
					prevValue = newValue; /* 前回の値を新しい値に更新 */ \
				} \
			ImGui::PopID(); \
		}

// ブール型プロパティ(name: 表示名, var: 変数, edited: 編集されたかどうかを返す変数)
#define IMGUI_PROPERTY_BOOL(name, var, edited) \
		{ \
			ImGui::PushID(&var); \
			IMGUI_PROPERTY(name) \
			edited |= ImGui::Checkbox("##bool", &var); \
			static bool prevValue; /* 前回の値を保持する静的変数 */ \
			if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
			{ \
				prevValue = var; \
			} \
			if (ImGui::IsItemDeactivatedAfterEdit()) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
				{ \
					bool newValue = var; \
					if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
					{ \
						IMGUI_PROPERTY_COMMAND_DEFAULT(name, var, newValue, prevValue) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
					} \
					prevValue = newValue; /* 前回の値を新しい値に更新 */ \
				} \
			ImGui::PopID(); \
		}
		

// 文字列型プロパティ(name: 表示名, var: std::string変数, bufferSize: バッファサイズ, edited: 編集されたかどうかを返す変数)※varはstd::stringで定義されている前提。ImGui::InputTextはCスタイルの文字列を扱うため、内部でバッファを用意してstd::stringの内容をコピーしてからInputTextに渡す必要がある。編集が確定したタイミングでCommandを作成してUndoRedoStackに追加するため、前回の値を保持する静的変数も用意している。
#define IMGUI_PROPERTY_STRING(name, var, bufferSize, edited) \
		{ \
			ImGui::PushID(&var); \
			char var##_buffer[bufferSize]; /* 変数はstd::stringで定義されている前提 */ \
			strncpy_s(var##_buffer, var.data(), bufferSize); /* 変数の内容をバッファにコピー */ \
			IMGUI_PROPERTY(name) \
			if (ImGui::InputText("##string", var##_buffer, bufferSize)) \
			{ \
				var = var##_buffer; /* 入力が変更されたら変数に反映 */ \
				edited = true; /* 編集されたフラグを立てる */ \
			}\
			static std::string prevValue; /* 前回の値を保持する静的変数 */ \
			 if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
			{ \
				prevValue = var; \
			} \
			if (ImGui::IsItemDeactivatedAfterEdit()) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
			{ \
				std::string newValue = var##_buffer; \
				if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
				{ \
					IMGUI_PROPERTY_COMMAND_CUSTOM(name, std::string(newValue), std::string(prevValue), std::string(newValue), std::string(prevValue), [this](const std::string& v) { var = v; }) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
				} \
				prevValue = newValue; /* 前回の値を新しい値に更新 */ \
			}\
			ImGui::PopID(); \
		}

// 拡張子生成
#define EXT_1(a) "*." #a "*"
#define EXT_2(a,b) "*." #a ";*." #b "*"
#define EXT_3(a,b,c) "*." #a ";*." #b ";*." #c "*"
#define EXT_4(a,b,c,d) "*." #a ";*." #b ";*." #c ";*." #d "*"
#define EXT_5(a,b,c,d,e) "*." #a ";*." #b ";*." #c ";*." #d ";*." #e "*"

// 引数カウント
#define GET_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME
#define EXT(...) GET_MACRO(__VA_ARGS__, EXT_5, EXT_4, EXT_3, EXT_2, EXT_1)(__VA_ARGS__)

// ダイアログフィルタ生成 (name: 表示名, ...: 拡張子)
// 例: FILTER("Image Files", png, jpg) -> "Image Files (*.png;*.jpg)\0*.png;*.jpg\0"
#define FILTER(name, ...) name " (" EXT(__VA_ARGS__) ")\0" EXT(__VA_ARGS__) "\0"

// ダイアログを開くボタン付き文字列プロパティ(name: 表示名, var: std::string変数, bufferSize: バッファサイズ, dialogFilter: ダイアログのファイルフィルタ, edited: 編集されたかどうかを返す変数)
// ※varはstd::stringで定義されている前提。ImGui::InputTextはCスタイルの文字列を扱うため、内部でバッファを用意してstd::stringの内容をコピーしてからInputTextに渡す必要がある。編集が確定したタイミングでCommandを作成してUndoRedoStackに追加するため、前回の値を保持する静的変数も用意している。ダイアログでファイルが選択された場合も同様にCommandを作成してUndoRedoStackに追加する。
#define IMGUI_PROPERTY_STRING_WITH_DIALOG(name, var, bufferSize, dialogFilter, edited) \
		ImGui::PushID(&var); \
		char var##_buffer[bufferSize]; /* 変数はstd::stringで定義されている前提 */ \
		strncpy_s(var##_buffer, var.data(), bufferSize); /* 変数の内容をバッファにコピー */ \
		IMGUI_PROPERTY(name) \
		if (ImGui::InputText("##string_with_dialog", var##_buffer, bufferSize)) \
		{ \
			var = var##_buffer; /* 入力が変更されたら変数に反映 */ \
			edited = true; /* 編集されたフラグを立てる */ \
		}\
		ImGui::SameLine(); \
		bool selected = false; /* ダイアログでファイルが選択されたかを追跡するフラグ */ \
		if (ImGui::Button("...")) { \
			char filepath[260] = {}; \
			if (Dialog::OpenFileName(filepath, sizeof(filepath), dialogFilter) == DialogResult::OK) { \
				var = filepath; /* ダイアログで選択されたファイルパスを変数に反映 */ \
				edited = true; /* 編集されたフラグを立てる */ \
				selected = true; \
			} \
		}\
		static std::string prevValue; /* 前回の値を保持する静的変数 */ \
		if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
		{ \
			prevValue = var; \
		} \
		if (ImGui::IsItemDeactivatedAfterEdit() || selected) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
		{ \
			std::string newValue = var; \
			if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
			{ \
				IMGUI_PROPERTY_COMMAND_CUSTOM(name, std::string(newValue), std::string(prevValue), std::string(newValue), std::string(prevValue), [this](const std::string& v) { var = v; }) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
			} \
			prevValue = newValue; /* 前回の値を新しい値に更新 */ \
		}\
		ImGui::PopID();

// 列挙型プロパティ(name: 表示名, var: 変数, items: アイテムの配列, itemCount: アイテム数)
#define IMGUI_PROPERTY_ENUM(name, var, items, itemCount) \
		ImGui::PushID(&var); \
		IMGUI_PROPERTY(name) \
		if (ImGui::BeginCombo("##enum", items[static_cast<size_t>(var)])) { \
			for (int n = 0; n < itemCount; n++) { \
				bool isSelected = (static_cast<int>(var) == n); \
				if (ImGui::Selectable(items[n], isSelected)) { \
					var = static_cast<decltype(var)>(n); /* 選択されたアイテムのインデックスを変数に反映 */ \
					auto newValue = var; \
					auto oldValue = static_cast<decltype(var)>(isSelected ? n : var); /* 変更前の値は、選択されているアイテムが現在の値と同じ場合はその値、そうでない場合は現在の値になる */ \
					if (newValue != oldValue) /* 値が変更された場合のみコマンドを追加 */ \
					{ \
						IMGUI_PROPERTY_COMMAND_ENUM(name, newValue, oldValue, items) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
					} \
				} \
				if (isSelected) { \
					ImGui::SetItemDefaultFocus(); /* 選択されているアイテムにフォーカスを当てる */ \
				} \
			} \
			ImGui::EndCombo(); \
		}\
		ImGui::PopID();


// ベクトル型プロパティ(name: 表示名, var: 変数, edited: 編集されたかどうかを返す変数, ...: ImGui::DragFloat2の引数)
#define IMGUI_PROPERTY_VECTOR2(name, var, edited, ...) \
		{ \
			ImGui::PushID(&var); \
			IMGUI_PROPERTY(name) \
			edited |= ImGui::DragFloat2("##vector2", reinterpret_cast<float*>(&var), ##__VA_ARGS__); \
			static Vector2 prevValue; /* 前回の値を保持する静的変数 */ \
			if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
			{ \
				prevValue = var; \
			} \
			if (ImGui::IsItemDeactivatedAfterEdit()) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
			{ \
				Vector2 newValue = var; \
				if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
				{ \
					IMGUI_PROPERTY_COMMAND_CUSTOM(name, var, newValue, prevValue, \
						"(" + std::to_string(newValue.x) + "," + std::to_string(newValue.y) + ")", \
						"(" + std::to_string(prevValue.x) + "," + std::to_string(prevValue.y) + ")", \
						[this](const Vector2& v) { var = v; }) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
				} \
				prevValue = newValue; /* 前回の値を新しい値に更新 */ \
			}\
			ImGui::PopID(); \
		}

// ベクトル型プロパティ(name: 表示名, var: 変数, edited: 編集されたかどうかを返す変数, ...: ImGui::DragFloat3の引数)
#define IMGUI_PROPERTY_VECTOR3(name, var, edited, ...) \
		{ \
			ImGui::PushID(&var); \
			IMGUI_PROPERTY(name) \
			edited |= ImGui::DragFloat3("##vector3", reinterpret_cast<float*>(&var), ##__VA_ARGS__); \
			static Vector3 prevValue; /* 前回の値を保持する静的変数 */ \
			if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
			{ \
				prevValue = var; \
			} \
			if (ImGui::IsItemDeactivatedAfterEdit()) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
			{ \
				Vector3 newValue = var; \
				if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
				{ \
					IMGUI_PROPERTY_COMMAND_CUSTOM(name, newValue, prevValue, \
						"(" + std::to_string(newValue.x) + "," + std::to_string(newValue.y) + "," + std::to_string(newValue.z) + ")", \
						"(" + std::to_string(prevValue.x) + "," + std::to_string(prevValue.y) + "," + std::to_string(prevValue.z) + ")", \
						[this](const Vector3& v) { var = v; }) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
				} \
				prevValue = newValue; /* 前回の値を新しい値に更新 */ \
			}\
			ImGui::PopID(); \
		}


// クォータニオン型プロパティ(name: 表示名, var: 変数, edited: 編集されたかどうかを返す変数, ...: ImGui::DragFloat4の引数)
#define IMGUI_PROPERTY_QUATERNION(name, var, edited, ...) \
		{ \
			ImGui::PushID(&var); \
			IMGUI_PROPERTY(name) \
			edited |= ImGui::DragFloat4("##quaternion", reinterpret_cast<float*>(&var), ##__VA_ARGS__); \
			static Quaternion prevValue; /* 前回の値を保持する静的変数 */ \
			if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
			{ \
				prevValue = var; \
			} \
			if (ImGui::IsItemDeactivatedAfterEdit()) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
			{ \
				Quaternion newValue = var; \
				if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
				{ \
					IMGUI_PROPERTY_COMMAND_CUSTOM(name, newValue, prevValue, \
						"(" + std::to_string(newValue.x) + "," + std::to_string(newValue.y) + "," + std::to_string(newValue.z) + "," + std::to_string(newValue.w) + ")", \
						"(" + std::to_string(prevValue.x) + "," + std::to_string(prevValue.y) + "," + std::to_string(prevValue.z) + "," + std::to_string(prevValue.w) + ")", \
						[this](const Quaternion& v) { var = v; }) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
				} \
				prevValue = newValue; /* 前回の値を新しい値に更新 */ \
			}\
			ImGui::PopID(); \
		}

// 色型プロパティ(name: 表示名, var: 変数, edited: 編集されたかどうかを返す変数)
#define IMGUI_PROPERTY_COLOR(name, var, edited) \
		{ \
			ImGui::PushID(&var); \
			IMGUI_PROPERTY(name) \
			edited |= ImGui::ColorEdit4("##color", reinterpret_cast<float*>(&var)); \
			static Color prevValue; /* 前回の値を保持する静的変数 */ \
			if (ImGui::IsItemActivated()) /* 編集開始時に前回の値を保存 */ \
			{ \
				prevValue = var; \
			} \
			if (ImGui::IsItemDeactivatedAfterEdit()) /* 確定したタイミングで、Commandを作成してUndoRedoStackに追加する */ \
			{ \
				Color newValue = var; \
				if (newValue != prevValue) /* 値が変更された場合のみコマンドを追加 */ \
				{ \
					IMGUI_PROPERTY_COMMAND_CUSTOM(name, newValue, prevValue, \
						"(" + std::to_string(newValue.r) + "," + std::to_string(newValue.g) + "," + std::to_string(newValue.b) + "," + std::to_string(newValue.a) + ")", \
						"(" + std::to_string(prevValue.r) + "," + std::to_string(prevValue.g) + "," + std::to_string(prevValue.b) + "," + std::to_string(prevValue.a) + ")", \
						[this](const Color& v) { var = v; }) /* 値が変更されたときにプロパティの値を更新するラムダ関数を渡す */ \
				} \
				prevValue = newValue; /* 前回の値を新しい値に更新 */ \
			}\
			ImGui::PopID(); \
		}

// コンポーネント参照プロパティ(name: 表示名, v: 参照するコンポーネントのObjectId変数, ownerV: 参照するコンポーネントの所有者のObjectId変数, tooltip: ツールチップとして表示する文字列)
#define IMGUI_PROPERTY_COMPONENT_REFERENCE(name, v, ownerV, tooltip) \
		{ \
			ImGui::PushID(&v); \
			IMGUI_PROPERTY(name) \
			{ \
				/*ドロップターゲットの設定*/ \
				if (ImGui::BeginDragDropTarget()) { \
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload((name).c_str())) { \
						ObjectId* droppedIdPtr = (ObjectId*)payload->Data; \
						ObjectId droppedId = *droppedIdPtr; \
						ObjectId droppedOwnerId = ScriptSystem::GetComponentOwner(*droppedIdPtr); \
						CurryEngine::History::ExecuteCommand( \
							std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, std::pair<ObjectId, ObjectId>>>>( \
								"Set " + name + " Component reference", \
								[this](const std::pair<std::string, std::pair<ObjectId, ObjectId>>& pair) { \
								std::string valueStr = "Component(objectId: " + std::to_string(pair.second.first.Value()) + ", ownerId: " + std::to_string(pair.second.second.Value()) + ")"; \
									ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(), valueStr); \
								}, \
								std::make_pair(name, std::make_pair(v, ownerV)), /* 変更前の値 */ \
								std::make_pair(name, std::make_pair(droppedId, droppedOwnerId)) /* 変更後の値 */ \
							) \
						); \
						/*スクリプトフィールドを更新する*/ \
						std::string valueStr = "Component(objectId: " + std::to_string(droppedId.Value()) + ", ownerId: " + std::to_string(droppedOwnerId.Value()) + ")"; \
						ScriptSystem::SetScriptField(m_gcHandle, name.c_str(), valueStr); \
						v = droppedId; \
						ownerV = droppedOwnerId; \
					} \
					ImGui::EndDragDropTarget(); \
				} \
				/*クリアボタン*/ \
				ImGui::SameLine(); \
				if (ImGui::Button("X")) {\
					CurryEngine::History::ExecuteCommand( \
						std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, std::pair<ObjectId, ObjectId>>>>( \
							"Clear " + name + " Component reference", \
							[this](const std::pair<std::string, std::pair<ObjectId, ObjectId>>& pair) { \
								std::string valueStr = "Component(objectId: " + std::to_string(pair.second.first.Value()) + ", ownerId: " + std::to_string(pair.second.second.Value()) + ")"; \
								ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(), valueStr); \
							}, \
							std::make_pair(name, std::make_pair(v, ownerV)), /* 変更前の値 */ \
							std::make_pair(name, std::make_pair(ObjectId::Invalid(), ObjectId::Invalid())) /* 変更後の値 */ \
						) \
					); \
					v = ObjectId::Invalid(); \
					ownerV = ObjectId::Invalid(); \
				}\
				if (!tooltip.empty() && ImGui::IsItemHovered()) \
					ImGui::SetTooltip("%s", tooltip.c_str()); \
			} \
			ImGui::PopID(); \
		}

// ゲームオブジェクト参照プロパティ(name: 表示名, v: 参照するゲームオブジェクトのObjectId変数, tooltip: ツールチップとして表示する文字列)
#define IMGUI_PROPERTY_GAMEOBJECT_REFERENCE(name, v, tooltip) \
		{ \
			ImGui::PushID(&v); \
			IMGUI_PROPERTY(name) \
			{ \
				/*ドロップターゲットの設定*/ \
				if (ImGui::BeginDragDropTarget()) { \
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) { \
						ObjectId* droppedIdPtr = (ObjectId*)payload->Data; \
						ObjectId droppedId = *droppedIdPtr; \
						CurryEngine::History::ExecuteCommand( \
							std::make_shared<CurryEngine::SetValueCommand<std::pair<std::string, ObjectId>>>( \
								"Set " + name + " GameObject reference", \
								[this](const std::pair<std::string, ObjectId>& pair) { \
									std::string valueStr = "GameObject(objectId: " + std::to_string(pair.second.Value()) + ")"; \
									ScriptSystem::SetScriptField(m_gcHandle, pair.first.c_str(), valueStr); \
								}, \
								std::make_pair(name, v), /* 変更前の値 */ \
								std::make_pair(name, droppedId) /* 変更後の値 */ \
							) \
						); \
						/*スクリプトフィールドを更新する*/ \
						std::string valueStr = "GameObject(objectId: " + std::to_string(droppedId.Value()) + ")"; \
						ScriptSystem::SetScriptField(m_gcHandle, name.c_str(), valueStr); \
						v = droppedId; \
					} \
					ImGui::EndDragDropTarget(); \
				}\
				if (!tooltip.empty() && ImGui::IsItemHovered()) \
					ImGui::SetTooltip("%s", tooltip.c_str());\
			}\
			ImGui::PopID(); \
		}



#endif // USE_IMGUI
