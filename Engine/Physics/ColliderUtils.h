#pragma once
#include "Collider.h"

class BoxCollider;
class SphereCollider;

/** @brief HitResult を表す構造体です。 */
struct HitResult {
	XMFLOAT3 hitPosition;
	XMFLOAT3 hitNormal;
};

/** @brief ColliderUtils を表すクラスです。 */
class ColliderUtils
{
public:
	/**
	 * @brief Intersect の処理を行います。
	 */
	static bool Intersect(const XMFLOAT3& point, BoxCollider* collider);
	/**
	 * @brief Intersect の処理を行います。
	 */
	static bool Intersect(const XMFLOAT3& point, SphereCollider* collider);
	/**
	 * @brief Intersect の処理を行います。
	 */
	static bool Intersect(BoxCollider* b0, BoxCollider* b1);
	/**
	 * @brief Intersect の処理を行います。
	 */
	static bool Intersect(BoxCollider* boxCollider, SphereCollider* sphereCollider);
	/**
	 * @brief Intersect の処理を行います。
	 */
	static bool Intersect(SphereCollider* s0, SphereCollider* s1);
	//レイキャスト（上面のみ判定）
	/**
	 * @brief Raycast の処理を行います。
	 */
	static bool Raycast(BoxCollider* collider, HitResult& hitResult);
	/**
	 * @brief Raycast の処理を行います。
	 */
	static bool Raycast(BoxCollider* collider, float& distance);

	/*
	 * @brief レイキャスト
	 * @param origin レイの始点
	 * @param direction レイの方向（正規化済み）
	 * @param collider 判定対象の BoxCollider
	 * @param hitResult ヒット情報の格納先
	 * @return ヒットしたかどうか
	 * @note 上面のみ判定
	 */
	static bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, BoxCollider* collider, HitResult& hitResult);

	/*
	 * @brief レイキャスト(円形平面との交差判定)
	 * @param origin レイの始点
	 * @param direction レイの方向（正規化済み）
	 * @param collider 判定対象の BoxCollider
	 * @param radius 円形平面の半径
	 * @param hitResult ヒット情報の格納先
	 * @return ヒットしたかどうか
	 */
	static bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, BoxCollider* collider, float radius, HitResult& hitResult);

	/*
	 * @brief レイキャスト(円形平面との交差判定。レイの方向は下方向固定)
	 * @param origin レイの始点
	 * @param center 円形平面の中心位置
	 * @param radius 円形平面の半径
	 * @param hitResult ヒット情報の格納先
	 * @return ヒットしたかどうか
	 */
	static bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& center, float radius, HitResult& hitResult);
};