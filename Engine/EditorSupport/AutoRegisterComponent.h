#pragma once
#include "ComponentFactory.h"

/**
 * @file
 * @brief コンポーネントの自動登録ヘルパー。
 * @details `ComponentFactory` へクラスを登録するためのテンプレート/マクロを提供します。
 *          生成関数やカテゴリ、拡張属性、依存要件をまとめて指定できます。
 */

#if 0
/**
 * @brief テンプレート版の自動登録（非推奨、参照用）。
 * @tparam T 登録するコンポーネント型。
 */
template<typename T>
class AutoRegisterComponent : public Component {
protected:
	static bool registered;
};

/**
 * @brief クラス `T` をファクトリに登録します。
 */
template<typename T>
bool AutoRegisterComponent<T>::registered =
[]() {
	ComponentFactory::Register(
		typeid(T).name(),
		[]()->std::shared_ptr<Component> { return std::make_shared<T>(); }
	);
	return true;
	}();

#else

/**
 * @brief 属性や要件付きでコンポーネントを登録するマクロ。
 * @param CLASS 登録対象のクラス名。
 * @param CATEGORY カテゴリ名（エディタなどの分類用）。
 * @param ATTRS 追加属性（`std::vector<AttributeBase*>` 相当を想定）。
 * @param REQS 依存要件（`std::vector<Requirement>` 等を想定）。
 * @details 静的レジストラを生成し、起動時に `ComponentFactory::Register` を呼び出します。
 */
#define REGISTER_COMPONENT_WITH_ATTRIBUTES(CLASS, CATEGORY, ATTRS, REQS) \
struct CLASS##Registrator { \
    CLASS##Registrator() { \
        ComponentFactory::Register( \
			#CLASS, \
			CATEGORY, \
			[]() -> std::shared_ptr<Component> { \
				return std::make_shared<CLASS>(); \
			}, \
			ATTRS, \
			REQS \
		); \
    } \
}; \
static inline CLASS##Registrator global_##CLASS##_registrator;

/**
 * @brief シンプルなコンポーネント登録マクロ。
 * @param CLASS 登録対象のクラス名。
 * @param CATEGORY カテゴリ名。
 * @details 生成関数のみを登録します。属性や要件が不要な場合に使用します。
 */
#define REGISTER_COMPONENT(CLASS, CATEGORY) \
static bool _registered_##CLASS = [](){ \
    ComponentFactory::Register(#CLASS, CATEGORY, []()-> std::shared_ptr<Component> { return std::make_shared<CLASS>(); }); \
    return true; \
}();

///**
// * @brief スクリプトコンポーネント専用の登録マクロ。
// * @param CLASS 登録対象のクラス名。
// * @details カテゴリを "Scripts" に固定して登録します。
// */
//#define REGISTER_SCRIPT_COMPONENT(CLASS) \
//static bool _registered_##CLASS = [](){ \
//    ComponentFactory::Register(#CLASS, "Scripts", []()-> std::shared_ptr<Component> { return std::make_shared<ScriptComponent>(); }); \
//    return true; \
//}();


#endif // 0

