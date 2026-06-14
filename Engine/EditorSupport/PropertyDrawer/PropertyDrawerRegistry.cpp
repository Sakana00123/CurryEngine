#include "pch.h"
#include "PropertyDrawerRegistry.h"

#include "Drawers/BoolDrawer.h"
#include "Drawers/IntDrawer.h"
#include "Drawers/FloatDrawer.h"
#include "Drawers/Vector2Drawer.h"
#include "Drawers/Vector3Drawer.h"
#include "Drawers/QuaternionDrawer.h"
#include "Drawers/EulerDrawer.h"
#include "Drawers/StringDrawer.h"
#include "Drawers/ColorDrawer.h"
#include "Drawers/ObjectIdDrawer.h"
#include "Drawers/AssetReferenceDrawer.h"
#include "Drawers/EnumDrawer.h"


namespace CurryEngine
{
	PropertyDrawerRegistry& PropertyDrawerRegistry::Get()
	{
		static PropertyDrawerRegistry instance;
		return instance;
	}

	PropertyDrawerRegistry::PropertyDrawerRegistry()
	{
		// TODO: あとで自動登録機能を実装する予定なので、現状は手動でドロワーを登録するためのコードをコンストラクタに書いています。
		Register("bool", std::make_unique<BoolDrawer>());
		Register("int", std::make_unique<IntDrawer>());
		Register("float", std::make_unique<FloatDrawer>());
		Register("Vector2", std::make_unique<Vector2Drawer>());
		Register("Vector3", std::make_unique<Vector3Drawer>());
		Register("Quaternion", std::make_unique<QuaternionDrawer>());
		Register("Quaternion_Euler", std::make_unique<EulerDrawer>());
		
		Register("std::string", std::make_unique<StringDrawer>());

		Register("Color", std::make_unique<ColorDrawer>());
		Register("ObjectId", std::make_unique<ObjectIdDrawer>());
		Register("String_AssetReference", std::make_unique<AssetReferenceDrawer>());
		// EnumDrawer は特定の型に依存しないため、"Enum" というキーで登録します。CustomDrawer 属性でこのキーを指定することで、任意の列挙型に対して EnumDrawer を使用できます。
		Register("Enum", std::make_unique<EnumDrawer>());
	}

	void PropertyDrawerRegistry::Register(const std::string& typeName, std::unique_ptr<IPropertyDrawer> drawer)
	{
		m_drawers[typeName] = std::move(drawer);
	}

	IPropertyDrawer* PropertyDrawerRegistry::Find(const std::string& typeName) const
	{
		auto it = m_drawers.find(typeName);
		if (it != m_drawers.end())
		{
			return it->second.get();
		}
		return nullptr; // 見つからない場合は nullptr を返す
	}
}