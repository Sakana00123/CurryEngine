#include "pch.h"
#include "ObjectId.h"
#include "Engine/Editor/Console.h"
#include <random>

ObjectId ObjectId::Generate()
{
	static std::mt19937_64 engine(std::random_device{}());
	static std::uniform_int_distribution<uint64_t> dist(IdRange::UUID_MIN, UINT64_MAX);
	return ObjectId(dist(engine));
}

ObjectId ObjectId::FromLegacy(int legacyId)
{
	if (legacyId == -1)
	{
		return ObjectId::Invalid(); // 古いIDの-1は無効なIDとして扱う
	}
	else if (legacyId <= 0 || static_cast<uint64_t>(legacyId) > IdRange::LEGACY_MAX)
	{
		// 旧IDが有効な範囲外の場合はエラーログを出力して無効なIDを返す
		Console::LogError("Invalid legacy ID: " + std::to_string(legacyId) + ". Must be between 1 and " + std::to_string(IdRange::LEGACY_MAX) + ".");
	}
	return ObjectId(static_cast<uint64_t>(legacyId));
}

std::string ObjectId::ToString() const
{
	return std::to_string(m_value);
}

ObjectId ObjectId::FromString(const std::string& str)
{
	try {
		uint64_t value = std::stoull(str);
		return ObjectId(value);
	}
	catch (const std::exception& e) {
		Console::LogError("Failed to parse ObjectId from string: " + str + ". Error: " + e.what());
		return ObjectId::Invalid();
	}
}

ObjectId ObjectId::FromValue(uint64_t value)
{
	return ObjectId(value);
}