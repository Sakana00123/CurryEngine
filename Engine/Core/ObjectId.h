#pragma once
#include <cstdint>
#include <string>

namespace IdRange {
	constexpr uint64_t LEGACY_MAX = 0x00000000FFFFFFFF; // 旧int範囲
	constexpr uint64_t UUID_MIN = 0x0001000000000000; // 新UUID範囲
}


class ObjectId
{
public:
	// デフォルトコンストラクタは無効なIDを生成する。内部のID値は0で、これは有効なIDとはみなされない。
	ObjectId() : m_value(0) {}
	explicit ObjectId(uint64_t v) : m_value(v) {}
	ObjectId(const ObjectId& other) = default;
	ObjectId& operator=(const ObjectId& other) = default;
	ObjectId(ObjectId&& other) noexcept = default;
	ObjectId& operator=(ObjectId&& other) noexcept = default;


	/// <summary>
	/// 新しいIDを生成する。旧int範囲を避け、UUID範囲から生成する。
	/// </summary>
	/// <returns>新しいObjectId</returns>
	static ObjectId Generate();

	/// <summary>
	/// 旧int範囲のIDからObjectIdを生成する。旧IDは新IDの一部として保持される。
	/// </summary>
	/// <param name="legacyId">旧int範囲のID</param>
	/// <returns>旧IDを保持するObjectId</returns>
	static ObjectId FromLegacy(int legacyId);

	/// <summary>
	/// 無効なIDを返す。内部のID値は0で、これは有効なIDとはみなされない。
	/// </summary>
	/// <returns>無効なObjectId</returns>
	static ObjectId Invalid() { return ObjectId(0); }

	// 内部のID値を取得する。UUID範囲のIDは旧IDを含むが、旧IDはUUID範囲のIDとは区別される。
	uint64_t Value() const { return m_value; }
	// IDが有効かどうかをチェックする。0は無効なIDとみなされる。
	bool IsValid() const { return m_value != 0; }
	// IDが旧int範囲のIDかどうかをチェックする。UUID範囲のIDは旧IDではない。
	bool IsLegacy() const { return m_value != 0 && m_value <= IdRange::LEGACY_MAX; }

	bool operator==(const ObjectId& other) const { return m_value == other.m_value; }

	/// <summary>
	/// IDを文字列に変換する。UUID範囲のIDは16進数で表現され、旧IDはそのまま10進数で表現される。
	/// </summary>
	/// <returns>IDの文字列表現</returns>
	std::string ToString() const;

	/// <summary>
	/// 文字列からIDを生成する。16進数の文字列はUUID範囲のIDとして解釈され、10進数の文字列は旧IDとして解釈される。
	/// </summary>
	/// <param name="str">IDの文字列表現</param>
	/// <returns>文字列から生成されたObjectId</returns>
	static ObjectId FromString(const std::string& str);

	/// <summary>
	/// IDの内部値からObjectIdを生成する。通常は使用しないが、特定の値を直接指定したい場合に便利。
	/// </summary>
	/// <param name="value">IDの内部値</param>
	/// <returns>内部値から生成されたObjectId</returns>
	static ObjectId FromValue(uint64_t value);

private:
	uint64_t m_value = 0;
};

namespace std {
	template <>
	struct hash<ObjectId>
	{
		size_t operator()(const ObjectId& id) const noexcept
		{
			return hash<uint64_t>()(id.Value());
		}
	};
}