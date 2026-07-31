#pragma once
#include <string>
#include <vector>
#include <any>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include "Engine/EditorSupport/AutoRegisterComponent.h"
#include "Engine/Editor/Console.h"

// C_REFLECT マクロをクラスに付けると、そのクラスのメタ情報が自動的に登録されるようになります。
#define C_REFLECT(ClassName) \
	friend struct ClassName##_AutoRegister;

// C_CLASS マクロをクラス宣言の前に付けると、そのクラスがリフレクションシステムに登録されます。引数で基底クラスも指定可能。
#define C_CLASS(...) \

// C_STRUCT マクロを struct 宣言の前に付けると、その構造体がリフレクションシステムに登録されます。引数で属性も指定可能。
#define C_STRUCT(...) \


/**
 * @brief C_PROPERTY マクロをプロパティ宣言の前に付けると、そのプロパティがリフレクションシステムに登録されます。
 * @details C_PROPERTY() の引数に属性を指定できます。例: `C_PROPERTY(Range(0, 100), HideInInspector)`
 * 		属性は `AttributeInfo` としてメタ情報に保存され、エディタなどで利用できます。
 * 		属性の引数は文字列として保存されるため、必要に応じてパースして利用してください。
 * 		属性の例: `Range(0, 100)`, `HideInInspector`, `Tooltip("説明")` など。
 */
#define C_PROPERTY(...) \
public:

 // C_FUNCTION マクロをメソッド宣言の前に付けると、そのメソッドがリフレクションシステムに登録されます。引数で属性も指定可能。
#define C_FUNCTION(...) \

// C_ENUM マクロを enum / enum class 宣言の前に付けると、その列挙型がリフレクションシステムに登録されます。引数で属性も指定可能。
#define C_ENUM(...) \



// ----------------- C_PROPERTYマクロ内に書ける属性 -----------------
namespace CurryEngine
{
	namespace PropertyAttributes
	{
		// プロパティをインスペクタに表示しない
		struct HideInInspector {};

		// プロパティをシリアライズ対象から除外する
		struct NonSerialized {};

		// プロパティをインスペクタで編集できないようにする
		struct ReadOnly {};

		// プロパティにツールチップを表示する
		struct Tooltip
		{
			const char* text;
			constexpr Tooltip(const char* t) : text(t) {}
		};

		// プロパティの値を指定した範囲内に制限する
		struct Range
		{
			float _min;
			float _max;
			constexpr Range(float min, float max) : _min(min), _max(max) {}
		};
		
		// プロパティの編集速度を指定する（例: ImGui の DragInt/DragFloat で使用）
		struct Speed
		{
			float value;
			constexpr Speed(float v) : value(v) {}
		};

		// プロパティが参照するオブジェクトの型を指定する（例: Component/GameObject など）。エディタでオブジェクト参照のドロップ操作をサポートするために使用します。
		struct ObjectReference
		{
			const char* targetType; // 参照先の型名 (例: "Component", "GameObject")
			constexpr ObjectReference(const char* target) : targetType(target) {}
		};

		// プロパティの表示フォーマットを指定する（例: "%.3f" など）。エディタでの数値表示に使用します。
		struct Format
		{
			const char* formatString; // 表示フォーマット (例: "%.3f" など)
			constexpr Format(const char* fmt) : formatString(fmt) {}
		};

		// ゲッターメソッドを指定する属性。C# のプロパティのように、フィールドではなくゲッターメソッドで値を取得する場合に使用します。
		struct Getter
		{
			const char* functionName; // ゲッターメソッドの名前 (例: "GetHealth")
			constexpr Getter(const char* fn) : functionName(fn) {}
		};

		// セッターメソッドを指定する属性。C# のプロパティのように、フィールドではなくセッターメソッドで値を設定する場合に使用します。
		struct Setter
		{
			const char* functionName; // セッターメソッドの名前 (例: "SetHealth")
			constexpr Setter(const char* fn) : functionName(fn) {}
		};

		// カスタムドロワーを指定する属性。エディタでプロパティの描画に使用するカスタムドロワーを指定します。
		struct CustomDrawer
		{
			const char* drawerType; // カスタムドロワーの種類を識別する文字列 (例: "Quaternion_Euler" など)。エディタでプロパティの描画に使用するカスタムドロワーを指定します。
			constexpr CustomDrawer(const char* dt) : drawerType(dt) {}
		};

		// ダイアログのフィルタを指定する属性。エディタでファイルダイアログを表示するときに使用します。
		struct DialogFilter
		{
			const char* filterString; // ダイアログのフィルタ文字列 (例: "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0")
			constexpr DialogFilter(const char* fs) : filterString(fs) {}
		};

		// 列挙型として扱うプロパティに付ける属性。エディタで列挙型のドロップダウンを表示するために使用します。
		struct Enum
		{
			const char* enumType; // 列挙型の型名 (例: "Color", "BlendMode" など)。エディタで列挙型のドロップダウンを表示するために使用します。
			constexpr Enum(const char* et) : enumType(et) {}
		};

		//struct ObjectPicker
		//{
		//	const char* targetType; // ピッカーで選択するオブジェクトの型名 (例: "Component", "GameObject")
		//	const char* filterString; // ピッカーのフィルタ文字列 (例: "All Objects (*.*)\0*.*\0")
		//	constexpr ObjectPicker(const char* target, const char* filter) : targetType(target), filterString(filter) {}
		//	ObjectPicker(const ObjectReference& objRef, const DialogFilter& dialogFilter)
		//		: targetType(objRef.targetType), filterString(dialogFilter.filterString) {
		//	}
		//};

	}
}

 // ---- リフレクションシステムのメタ情報構造体と登録システム ----

// ---- メタ情報構造体 ----

// 属性情報
struct AttributeInfo
{
	std::string name;					// 属性名 (例: "HideInInspector", "Range", "Tooltip")
	std::vector<std::string> args{};	// 属性引数 (例: Range(0, 100) なら args = {"0", "100"}, Tooltip("説明") なら args = {"説明"})
};

// プロパティのメタ情報
struct PropertyInfo
{
	std::string type;
	std::string name;
	size_t offset = 0; // クラス内のオフセット (C++のメンバ変数のアドレスを計算するために使用。C#では不要のため0のまま)
	std::vector<AttributeInfo> attributes{};
	bool hasCustomGetter = false; // ゲッターメソッドが指定されているかどうか
	bool hasCustomSetter = false; // セッターメソッドが指定されているかどうか

	// --- アクセサ (C++はoffsetを使って直接アクセス、C#は P / Invoke ラムダ式でアクセス) ---
	std::function<std::any(void* instance)> getter; // プロパティの値を取得する関数オブジェクト。引数はインスタンスポインタで、戻り値は any。
	std::function<void(void* instance, std::any value)> setter; // プロパティの値を設定する関数オブジェクト。引数は (インスタンスポインタ, 設定する値のany) で、戻り値は void。

	// 指定した属性情報を取得する関数(無かったらnullptrを返す)
	const AttributeInfo* GetAttribute(const std::string& attrName) const {
		for (auto& attr : attributes) {
			if (attr.name == attrName) {
				return &attr;
			}
		}
		return nullptr;
	}
};

// メソッドのメタ情報
struct MethodInfo
{
	std::string returnType;
	std::string name;
	std::vector<std::pair<std::string, std::string>> parameters; // (型, 名前)のペア
	std::vector<AttributeInfo> attributes{};

	// メソッド呼び出し用の関数オブジェクト。引数は (インスタンスポインタ, 引数のanyベクター) で、戻り値は any。
	std::function<std::any(void* instance, std::vector<std::any> args)> invoker;

	// 戻り値あり
	std::any Invoke(void* instance, std::vector<std::any> args = {}) const;

	// 戻り値なし（void）
	void InvokeVoid(void* instance, std::vector<std::any> args = {}) const;

	// 戻り値を型指定してキャスト
	template <typename TRet>
	TRet InvokeAs(void* instance, std::vector<std::any> args = {}) const {
		std::any result = Invoke(instance, args);
		if (result.has_value())
		{
			try
			{
				return std::any_cast<TRet>(result);
			}
			catch (const std::bad_any_cast& e)
			{
				LOG_ERROR("Failed to cast method return value: " + std::string(e.what()));
			}
		}
		else
		{
			LOG_ERROR("Method did not return a value.");
		}
	}
};

// クラスのメタ情報
struct ClassMeta
{
	std::string name;
	std::vector<std::string> bases;
	std::vector<PropertyInfo> properties;
	std::vector<MethodInfo> methods;
	bool isScript = false; // スクリプトクラスかどうか (エディタでスクリプトクラスを特別扱いするために使用)

	// 基底クラスを再帰的に検索してプロパティを取得する関数
	const PropertyInfo* FindProperty(const std::string& propName) const;

	// 基底クラスを再帰的に検索してメソッドを取得する関数
	const MethodInfo* FindMethod(const std::string& methodName) const;
};

// 列挙型の値のメタ情報
struct EnumValueInfo
{
	std::string name;
	int         value = 0;
	bool        hasExplicitValue = false; // 値が明示されているか
};

// 列挙型のメタ情報
struct EnumInfo
{
	std::string name;
	std::string underlyingType = "int"; // enum class : uint など
	bool        isClass = false; // enum class かどうか
	std::vector<EnumValueInfo> values;
};

// 構造体のメタ情報
struct StructInfo
{
	std::string name;
	std::vector<PropertyInfo> properties;
};

// ---- リフレクション登録システム ----
class ReflectionRegistry
{
public:
	// クラス登録
	static void Register(const ClassMeta& meta);
	// 列挙型登録
	static void RegisterEnum(const EnumInfo& meta);
	// 構造体登録
	static void RegisterStruct(const StructInfo& meta);
	// クラス検索
	static const ClassMeta* FindClass(const std::string& name);
	// 列挙型検索
	static const EnumInfo* FindEnum(const std::string& name);
	// 構造体検索
	static const StructInfo* FindStruct(const std::string& name);
	// 全スクリプトのメタ情報をクリア
	static void UnregisterScriptClasses();
private:
	static std::unordered_map<std::string, ClassMeta>& GetClassRegistry();
	static std::unordered_map<std::string, EnumInfo>& GetEnumRegistry();
	static std::unordered_map<std::string, StructInfo>& GetStructRegistry();
};

// -------------------------- ヘルパー関数 ---------------------

// 汎用ヘルパー
template <typename T>
T AnyCast(const std::any& a)
{
	return std::any_cast<T>(a);
}

// メンバ関数ポインタ → invoker 変換
template <typename TClass, typename TRet, typename... TArgs, std::size_t... I>
auto MakeInvokerImpl(TRet (TClass::*fn)(TArgs...), std::index_sequence<I...>)
{
	return [fn](void* instance, std::vector<std::any> args) -> std::any
	{
		TClass* obj = static_cast<TClass*>(instance);
		if constexpr (std::is_void_v<TRet>)
		{
			(obj->*fn)(AnyCast<TArgs>(args[I])...);
			return {};
		}
		else
		{
			return (obj->*fn)(AnyCast<TArgs>(args[I])...);
		}
	};
}
// const メンバ関数オーバーロード
template <typename TClass, typename TRet, typename... TArgs, std::size_t... I>
auto MakeInvokerImpl(TRet (TClass::*fn)(TArgs...) const, std::index_sequence<I...>)
{
	return [fn](void* instance, std::vector<std::any> args) -> std::any
	{
		const TClass* obj = static_cast<const TClass*>(instance);
		if constexpr (std::is_void_v<TRet>)
		{
			(obj->*fn)(AnyCast<TArgs>(args[I])...);
			return {};
		}
		else
		{
			return (obj->*fn)(AnyCast<TArgs>(args[I])...);
		}
	};
}

// メンバ関数ポインタ → invoker 変換のエントリーポイント
template <typename TClass, typename TRet, typename... TArgs>
auto MakeInvoker(TRet (TClass::*fn)(TArgs...))
{
	return MakeInvokerImpl(fn, std::index_sequence_for<TArgs...>{});
}
// const メンバ関数オーバーロード
template <typename TClass, typename TRet, typename... TArgs>
auto MakeInvoker(TRet (TClass::*fn)(TArgs...) const)
{
	return MakeInvokerImpl(fn, std::index_sequence_for<TArgs...>{});
}

// -------------------------- マクロ定義 ---------------------

// ---- マクロ ----
#if 0
#define REGISTER_CLASS(ClassName, BaseName) \
    namespace { struct ClassName##_AutoRegister { \
        static size_t GetOffset(auto ClassName::* member) \
        { \
            return reinterpret_cast<size_t>( \
                &(reinterpret_cast<ClassName*>(0)->*member) \
            ); \
        } \
        ClassName##_AutoRegister() { \
            ClassMeta meta; \
            meta.name = #ClassName; \
            meta.base = #BaseName;  

#endif // 0


#define REGISTER_CLASS(ClassName, ...) \
    namespace { struct ClassName##_AutoRegister { \
        static size_t GetOffset(auto ClassName::* member) \
        { \
            return reinterpret_cast<size_t>( \
                &(reinterpret_cast<ClassName*>(0)->*member) \
            ); \
        } \
        ClassName##_AutoRegister() { \
            ClassMeta meta; \
            meta.name = #ClassName; \
            meta.bases = []{ \
                std::vector<std::string> v; \
                std::string s = #__VA_ARGS__; \
                /* カンマ区切りで分割 */ \
                std::istringstream ss(s); std::string t; \
                while (std::getline(ss, t, ',')) { \
                    auto b = t.find_first_not_of(" \t"); \
                    auto e = t.find_last_not_of(" \t"); \
                    if (b != std::string::npos) v.push_back(t.substr(b, e-b+1)); \
                } \
                return v; \
            }();


#define REGISTER_PROPERTY(ClassName, propName, propType) \
    { \
        PropertyInfo p; \
        p.type   = #propType; \
        p.name   = #propName; \
        p.offset = GetOffset(&ClassName::propName); \
		p.attributes = {}; /* 属性は空のまま */ \
        p.getter = [](void* inst) -> std::any { \
            return static_cast<ClassName*>(inst)->propName; \
        }; \
        p.setter = [](void* inst, std::any val) { \
            static_cast<ClassName*>(inst)->propName = std::any_cast<propType>(val); \
        }; \
        meta.properties.push_back(p); \
    }

#define REGISTER_METHOD(ClassName, MethodName, ReturnType, CastExpr, ...)                                                           \
	{                                                                                                                     \
		MethodInfo m;                                                                                                     \
		m.name = #MethodName;                                                                                             \
		m.returnType = #ReturnType;                                                                                       \
		std::string _rm_argsStr = #__VA_ARGS__;                                                                           \
		std::istringstream _rm_ss(_rm_argsStr);                                                                           \
		std::string _rm_arg;                                                                                              \
		while (std::getline(_rm_ss, _rm_arg, ','))                                                                        \
		{                                                                                                                 \
			auto _rm_b = _rm_arg.find_first_not_of(" \t");                                                                \
			auto _rm_e = _rm_arg.find_last_not_of(" \t");                                                                 \
			if (_rm_b != std::string::npos)                                                                               \
			{                                                                                                             \
				std::string _rm_trimmed = _rm_arg.substr(_rm_b, _rm_e - _rm_b + 1);                                       \
				size_t _rm_spacePos = _rm_trimmed.rfind(' ');                                                             \
				if (_rm_spacePos != std::string::npos)                                                                    \
				{                                                                                                         \
					m.parameters.emplace_back(_rm_trimmed.substr(0, _rm_spacePos), _rm_trimmed.substr(_rm_spacePos + 1)); \
				}                                                                                                         \
				else                                                                                                      \
				{                                                                                                         \
					m.parameters.emplace_back(_rm_trimmed, "");                                                           \
				}                                                                                                         \
			}                                                                                                             \
		}                                                                                                                 \
		m.invoker = MakeInvoker(##CastExpr);																				  \
		meta.methods.push_back(m);                                                                                        \
	}

#define ATTR(name, ...) AttributeInfo{ #name, { __VA_ARGS__ } }

#define REGISTER_PROPERTY_WITH_ATTR(ClassName, propName, propType, ...) \
		{ \
            PropertyInfo p; \
			p.type = #propType; \
			p.name = #propName; \
			p.offset = GetOffset(&ClassName::propName); \
			p.attributes = std::vector<AttributeInfo>{ __VA_ARGS__ }; \
			p.getter = [](void* inst) -> std::any { \
			    return static_cast<ClassName*>(inst)->propName; \
			}; \
			p.setter = [](void* inst, std::any val) { \
			    static_cast<ClassName*>(inst)->propName = std::any_cast<propType>(val); \
			}; \
			meta.properties.push_back(p); \
		}

#define REGISTER_PROPERTY_WITH_CUSTOM_GETTER(ClassName, propName, propType, GetterFunc, ...) \
		{ \
			PropertyInfo p; \
			p.type = #propType; \
			p.name = #propName; \
			p.offset = GetOffset(&ClassName::propName); \
			p.attributes = std::vector<AttributeInfo>{ __VA_ARGS__  }; \
			p.getter = [](void* inst) -> std::any { \
				return static_cast<ClassName*>(inst)->GetterFunc(); \
			}; \
			p.setter = [](void* inst, std::any val) { \
				static_cast<ClassName*>(inst)->propName = std::any_cast<propType>(val); /* セッターは通常通りフィールドにアクセス */ \
			}; \
			p.hasCustomGetter = true; /* ゲッターメソッドが指定されていることを示すフラグ */ \
			meta.properties.push_back(p); \
		}

#define REGISTER_PROPERTY_WITH_CUSTOM_SETTER(ClassName, propName, propType, SetterFunc, ...) \
		{ \
			PropertyInfo p; \
			p.type = #propType; \
			p.name = #propName; \
			p.offset = GetOffset(&ClassName::propName); \
			p.attributes = std::vector<AttributeInfo>{ __VA_ARGS__  }; \
			p.getter = [](void* inst) -> std::any { \
				return static_cast<ClassName*>(inst)->propName; /* ゲッターは通常通りフィールドにアクセス */ \
			}; \
			p.setter = [](void* inst, std::any val) { \
				static_cast<ClassName*>(inst)->SetterFunc(std::any_cast<propType>(val)); \
			}; \
			p.hasCustomSetter = true; /* セッターメソッドが指定されていることを示すフラグ */ \
			meta.properties.push_back(p); \
		}

#define REGISTER_PROPERTY_WITH_CUSTOM_ACCESSOR(ClassName, propName, propType, GetterFunc, SetterFunc, ...) \
		{ \
			PropertyInfo p; \
			p.type = #propType; \
			p.name = #propName; \
			p.offset = GetOffset(&ClassName::propName); \
			p.attributes = std::vector<AttributeInfo>{ __VA_ARGS__ }; \
			p.getter = [](void* inst) -> std::any { \
				return static_cast<ClassName*>(inst)->GetterFunc(); \
			}; \
			p.setter = [](void* inst, std::any val) { \
				static_cast<ClassName*>(inst)->SetterFunc(std::any_cast<propType>(val)); \
			}; \
			p.hasCustomGetter = true; /* ゲッターメソッドが指定されていることを示すフラグ */ \
			p.hasCustomSetter = true; /* セッターメソッドが指定されていることを示すフラグ */ \
			meta.properties.push_back(p); \
		}

#define END_REGISTER(ClassName) \
            ReflectionRegistry::Register(meta); \
        } \
    } ClassName##_AutoRegisterInstance; };

// 列挙型登録マクロ

#define REGISTER_ENUM(EnumName) \
	namespace { struct EnumName##_AutoRegister { \
		EnumName##_AutoRegister() { \
			EnumInfo meta; \
			meta.name = #EnumName; \
			static int _enumValueCounter = 0; /* 値が明示されていない場合の自動カウンター */

#define UNDERLYING_TYPE(Type) \
			meta.underlyingType = #Type;

#define ENUM_VALUE(ValueName, ...) \
			{ \
				EnumValueInfo v; \
				v.name = ValueName; \
				if constexpr (sizeof(#__VA_ARGS__) > 1) /* 値が明示されている場合 */ \
				{ \
					v.value = __VA_ARGS__; \
					v.hasExplicitValue = true; \
					_enumValueCounter = v.value + 1; /* カウンターを明示された値の次に更新 */ \
				} \
				else /* 値が明示されていない場合は自動カウンターを使用 */ \
				{ \
					v.value = _enumValueCounter++; \
					v.hasExplicitValue = false; \
				} \
				meta.values.push_back(v); \
			}

#define END_REGISTER_ENUM(EnumName) \
			ReflectionRegistry::RegisterEnum(meta); \
		} \
	} EnumName##_AutoRegisterInstance; };


// 構造体登録マクロ
#define REGISTER_STRUCT(StructName) \
	namespace { struct StructName##_AutoRegister { \
		static size_t StructMetaOffset(auto StructName::* member) \
		{ \
			return reinterpret_cast<size_t>( \
				&(reinterpret_cast<StructName*>(0)->*member) \
			); \
		} \
		StructName##_AutoRegister() { \
			StructInfo meta; \
			meta.name = #StructName;

#define STRUCT_FIELD(StructName, FieldName, FieldType) \
			{ \
				PropertyInfo p; \
				p.type = #FieldType; \
				p.name = #FieldName; \
				p.offset = StructMetaOffset(&StructName::FieldName); \
				p.attributes = {}; \
				p.getter = [](void* inst) -> std::any { \
					return static_cast<StructName*>(inst)->FieldName; \
				}; \
				p.setter = [](void* inst, std::any val) { \
					static_cast<StructName*>(inst)->FieldName = std::any_cast<FieldType>(val); \
				}; \
				meta.properties.push_back(p); \
			}

#define END_REGISTER_STRUCT(StructName) \
			ReflectionRegistry::RegisterStruct(meta); \
		} \
	} StructName##_AutoRegisterInstance; };