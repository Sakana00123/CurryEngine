#pragma once
#include "json.hpp"
using namespace nlohmann;
#include "Engine/Core/ObjectId.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Core/Math/Quaternion.h"
#include "Engine/Core/Color.h"
#include "Engine/Types/Range.h"
#include "Engine/Core/Reflection/TypeSerializerRegistry.h"
#include "Engine/Resources/AssetId.h"
//#include "Engine/Core/Reference.h"

namespace CurryEngine
{
	template<typename T>
	void from_json(const nlohmann::json & j, Range<T>&r) {
		if (j.is_array() && j.size() == 2) {
			r.min = j.at(0).get<T>();
			r.max = j.at(1).get<T>();
		}
	}
	template<typename T>
	void to_json(nlohmann::json & j, const Range<T>&r) {
		j = nlohmann::json::array({ r.min, r.max });
	}
}

//template<typename T>
//void from_json(const json& j, Reference<T>& ref) {
//	if (j.is_string()) {
//		ref.id = ObjectId::FromString(j.get<std::string>());
//	}
//	else if (j.is_number_integer()) {
//		ref.id = ObjectId::FromLegacy(j.get<int>());
//	}
//	else {
//		ref.id = ObjectId::Invalid();
//	}
//}
//
//template<typename T>
//void to_json(json& j, const Reference<T>& ref) {
//	if (ref.IsValid()) {
//		j = ref.id.ToString();
//	}
//	else {
//		j = ObjectId::Invalid().ToString(); // 無効なIDは常に同じ文字列になる
//	}
//}

namespace CurryEngine
{
	namespace Resources
	{
		template<typename T>
		void from_json(const json& j, std::vector<T>& vec) {
			if (j.is_array()) {
				vec.clear();
				for (const auto& item : j) {
					vec.push_back(item.get<T>());
				}
			}
		}
		template<typename T>
		void to_json(json& j, const std::vector<T>& vec) {
			j = json::array();
			for (const auto& item : vec) {
				j.push_back(item);
			}
		}
		C_REGISTER_TYPE(AssetId);
	}
}

inline void from_json(const json& j, ObjectId& id) {
	if (j.is_string()) {
		id = ObjectId::FromString(j.get<std::string>());
	}
	else if (j.is_number_integer()) {
		id = ObjectId::FromLegacy(j.get<int>());
	}
	else {
		id = ObjectId::Invalid();
	}
}
inline void to_json(json& j, const ObjectId& id) {
	if (id.IsValid()) {
		j = id.ToString();
	}
	else {
		j = ObjectId::Invalid().ToString(); // 無効なIDは常に同じ文字列になる
	}
}
C_REGISTER_TYPE(ObjectId);

inline void from_json(const json& j, Vector2& v) {
	if (j.is_array() && j.size() == 2) {
		j[0].get_to(v.x);
		j[1].get_to(v.y);
	}
}
inline void to_json(json& j, const Vector2& v) {
	j = json::array({ v.x, v.y });
}
C_REGISTER_TYPE(Vector2);

inline void from_json(const json& j, Vector3& v) {
	if (j.is_array() && j.size() == 3) {
		j[0].get_to(v.x);
		j[1].get_to(v.y);
		j[2].get_to(v.z);
	}
}
inline void to_json(json& j, const Vector3& v) {
	j = json::array({ v.x, v.y, v.z });
}
C_REGISTER_TYPE(Vector3);

inline void from_json(const json& j, Color& c) {
	if (j.is_array() && j.size() == 4) {
		j[0].get_to(c.r);
		j[1].get_to(c.g);
		j[2].get_to(c.b);
		j[3].get_to(c.a);
	}
}
inline void to_json(json& j, const Color& c) {
	j = json::array({ c.r, c.g, c.b, c.a });
}
C_REGISTER_TYPE(Color);

namespace DirectX
{
	inline void from_json(const json& j, XMFLOAT2& v) {
		if (j.is_array() && j.size() == 2) {
			j[0].get_to(v.x);
			j[1].get_to(v.y);
		}
	}
	inline void to_json(json& j, const XMFLOAT2& v) {
		j = json::array({ v.x, v.y });
	}
	C_REGISTER_TYPE(XMFLOAT2);
	C_REGISTER_TYPE(DirectX::XMFLOAT2);

	inline void from_json(const json& j, XMFLOAT3& v) {
		if (j.is_array() && j.size() == 3) {
			j[0].get_to(v.x);
			j[1].get_to(v.y);
			j[2].get_to(v.z);
		}
	}
	inline void to_json(json& j, const XMFLOAT3& v) {
		j = json::array({ v.x, v.y, v.z });
	}
	C_REGISTER_TYPE(XMFLOAT3);
	C_REGISTER_TYPE(DirectX::XMFLOAT3);

	inline void from_json(const json& j, XMFLOAT4& v) {
		if (j.is_array() && j.size() == 4) {
			j[0].get_to(v.x);
			j[1].get_to(v.y);
			j[2].get_to(v.z);
			j[3].get_to(v.w);
		}
	}
	inline void to_json(json& j, const XMFLOAT4& v) {
		j = json::array({ v.x, v.y, v.z, v.w });
	}
	C_REGISTER_TYPE(XMFLOAT4);
	C_REGISTER_TYPE(DirectX::XMFLOAT4);
	C_REGISTER_TYPE(Quaternion);
}
