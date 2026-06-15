#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "IPropertyDrawer.h"

namespace CurryEngine
{
	/**
	 * @brief プロパティドロワーのレジストリクラス。プロパティの型に対応するドロワーを管理し、プロパティ描画関数を提供します。
	 * @details PropertyDrawerRegistry は、プロパティの型名をキーとして、対応する IPropertyDrawer のインスタンスを管理するクラスです。プロパティ描画の際に、プロパティの型に対応するドロワーをこのレジストリから取得して使用します。
	 * @note 現在はシングルトンパターンで実装されていますが、将来的に複数インスタンスが必要になった場合はシングルトンをやめる予定です。
	 */
	class PropertyDrawerRegistry
	{
	public:
		// TODO: 将来的に複数インスタンスが必要になったらシングルトンをやめる
		static PropertyDrawerRegistry& Get();
		// コンストラクタでドロワーの登録を行う
		// TODO: あとで自動登録機能を実装する予定なので、現状は手動でドロワーを登録するためのコードをコンストラクタに書いています。
		PropertyDrawerRegistry();

		/**
		 * @brief ドロワーの登録。プロパティの型名をキーとして、対応する IPropertyDrawer のインスタンスをレジストリに登録します。
		 * @param typeName ドロワーを登録するプロパティの型名。例: "int", "float", "Vector3" など。
		 * @param drawer 登録する IPropertyDrawer のインスタンス。プロパティの型に対応する描画処理を実装したクラスのインスタンスを渡してください。
		 * @note ドロワーは std::unique_ptr で管理されるため、呼び出し元はドロワーの所有権を渡す必要があります。例: `registry.Register("int", std::make_unique<IntPropertyDrawer>());`
		 */
		void Register(const std::string& typeName, std::unique_ptr<IPropertyDrawer> drawer);

		// ドロワーの取得。見つからない場合は nullptr を返す
		IPropertyDrawer* Find(const std::string& typeName) const;
	private:
		std::unordered_map<std::string, std::unique_ptr<IPropertyDrawer>> m_drawers;
	};
}