#pragma once
#include <string>
#include <any>
#include <json.hpp>

namespace CurryEngine
{
	namespace ScriptFieldSerializer
	{
		inline std::string ToJson(const std::any& value)
		{
			if (value.type() == typeid(int))
				return std::to_string(std::any_cast<int>(value));
			else if (value.type() == typeid(float))
				return std::to_string(std::any_cast<float>(value));
			else if (value.type() == typeid(double))
				return std::to_string(std::any_cast<double>(value));
			else if (value.type() == typeid(long))
				return std::to_string(std::any_cast<long>(value));
			else if (value.type() == typeid(unsigned long))
				return std::to_string(std::any_cast<unsigned long>(value));
			else if (value.type() == typeid(unsigned int))
				return std::to_string(std::any_cast<unsigned int>(value));
			else if (value.type() == typeid(short))
				return std::to_string(std::any_cast<short>(value));
			else if (value.type() == typeid(unsigned short))
				return std::to_string(std::any_cast<unsigned short>(value));
			else if (value.type() == typeid(char))
				return std::to_string(std::any_cast<char>(value));
			else if (value.type() == typeid(unsigned char))
				return std::to_string(std::any_cast<unsigned char>(value));
			else if (value.type() == typeid(long long))
				return std::to_string(std::any_cast<long long>(value));
			else if (value.type() == typeid(unsigned long long))
				return std::to_string(std::any_cast<unsigned long long>(value));
			else if (value.type() == typeid(bool))
				return std::any_cast<bool>(value) ? "true" : "false";
			else if (value.type() == typeid(std::string))
				return "\"" + std::any_cast<std::string>(value) + "\"";
			
			if (auto* v = std::any_cast<std::array<float, 2>>(&value))
				return "{\"x\":" + std::to_string((*v)[0])
					 + ",\"y\":" + std::to_string((*v)[1]) + "}";
			if (auto* v = std::any_cast<std::array<float, 3>>(&value))
				return "{\"x\":" + std::to_string((*v)[0])
					 + ",\"y\":" + std::to_string((*v)[1])
					 + ",\"z\":" + std::to_string((*v)[2]) + "}";
			if (auto* v = std::any_cast<std::array<float, 4>>(&value))
				return "{\"x\":" + std::to_string((*v)[0])
					 + ",\"y\":" + std::to_string((*v)[1])
					 + ",\"z\":" + std::to_string((*v)[2])
					 + ",\"w\":" + std::to_string((*v)[3]) + "}";

			return "\"[Unsupported:" + std::string(value.type().name()) + "]\"";
		}

		inline std::any FromJson(const std::string& typeStr, const nlohmann::json& j)
		{
			if (typeStr == "int" || typeStr == "Int32")
				return j.get<int>();
			else if (typeStr == "float" || typeStr == "Single")
				return j.get<float>();
			else if (typeStr == "double" || typeStr == "Double")
				return j.get<double>();
			else if (typeStr == "long" || typeStr == "Int64")
				return j.get<long>();
			else if (typeStr == "unsigned long" || typeStr == "ulong")
				return j.get<unsigned long>();
			else if (typeStr == "unsigned int" || typeStr == "uint")
				return j.get<unsigned int>();
			else if (typeStr == "short" || typeStr == "Int16")
				return j.get<short>();
			else if (typeStr == "unsigned short" || typeStr == "ushort")
				return j.get<unsigned short>();
			else if (typeStr == "char" || typeStr == "Char")
				return j.get<char>();
			else if (typeStr == "unsigned char" || typeStr == "uchar")
				return j.get<unsigned char>();
			else if (typeStr == "long long" || typeStr == "longlong")
				return j.get<long long>();
			else if (typeStr == "unsigned long long" || typeStr == "ulonglong")
				return j.get<unsigned long long>();
			else if (typeStr == "bool" || typeStr == "Boolean")
				return j.get<bool>();
			else if (typeStr == "std::string" || typeStr == "string")
				return j.get<std::string>();
			if (typeStr == "Vector2")
			{
				std::array<float, 2> v;
				v[0] = j["x"].get<float>();
				v[1] = j["y"].get<float>();
				return v;
			}
			if (typeStr == "Vector3")
			{
				std::array<float, 3> v;
				v[0] = j["x"].get<float>();
				v[1] = j["y"].get<float>();
				v[2] = j["z"].get<float>();
				return v;
			}
			if (typeStr == "Vector4" || typeStr == "Quaternion")
			{
				std::array<float, 4> v;
				v[0] = j["x"].get<float>();
				v[1] = j["y"].get<float>();
				v[2] = j["z"].get<float>();
				v[3] = j["w"].get<float>();
				return v;
			}

			throw std::runtime_error("Unsupported type for deserialization: " + typeStr);
			return std::any();
		}

	}
}