#pragma once
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include "Engine/Editor/Console.h"

class Component;

namespace ComponentAttributes
{
    enum : unsigned int {
        None = 0,
        DisallowMultiple = 1 << 0, // 同一ゲームオブジェクトに複数存在できない
        ExecuteInEditMode = 1 << 1, // エディタモードでも Update を呼び出す
		HideInAddComponentMenu = 1 << 2, // Add Component メニューに表示しない
		RequiredComponent = 1 << 3, // 指定したコンポーネントが必要（RequireComponent）
    };
}

/** @brief コンポーネントのファクトリークラス。
 * @details コンポーネントの登録と生成を管理します。属性フラグや依存関係もサポートします。
 */
class ComponentFactory {
public:
    struct Entry {
        std::string category;
        std::function<std::shared_ptr<Component>()> createFunc;

        // 追加部分：制約系属性
        unsigned int attributes = 0;                  // DisallowMultiple, ExecuteInEditMode など
        std::vector<std::string> requiredComponents; // RequireComponent 用
    };

    static void Register(
        const std::string& name,
        const std::string& category,
        std::function<std::shared_ptr<Component>()> func,
        unsigned int attributes = 0,
        std::vector<std::string> requireComponents = {}
    );

	/** @brief 指定の名前のコンポーネントを生成します。
	 * @param name コンポーネント名。
	 * @return 生成されたコンポーネントの共有ポインタ。名前が不明な場合は nullptr。
     */
    static std::shared_ptr<Component> Create(const std::string& name);

	/** @brief 指定の名前のコンポーネントが登録されているかを返します。
	 *  @param name コンポーネント名。
	 *  @return 登録されていれば true、そうでなければ false。
     */
    static bool Exists(const std::string& name);

	/** @brief 登録されているすべてのコンポーネント情報を返します。
	 * @return コンポーネント名をキー、Entry を値とするマップ。
     */
    static std::unordered_map<std::string, Entry>& GetAll();

private:
	static std::unordered_map<std::string, Entry>& Registry();
};