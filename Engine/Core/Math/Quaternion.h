#pragma once
#include <DirectXMath.h>
#include "Vector3.h"

using namespace DirectX;

/** @brief Quaternion を表す構造体です。 */
struct Quaternion : public XMFLOAT4
{
	Quaternion() : XMFLOAT4(0, 0, 0, 1) {}
	Quaternion(float x, float y, float z, float w) : XMFLOAT4(x, y, z, w) {}
	Quaternion(_In_reads_(4) const float* pArray) : XMFLOAT4(pArray) {}
	/**
	 * @brief Quaternion を構築します。
	 */
	Quaternion(const Quaternion&) = default;
	/**
	 * @brief 演算子処理を行います。
	 */
	Quaternion& operator=(const Quaternion&) = default;
	/**
	 * @brief Quaternion を構築します。
	 */
	Quaternion(Quaternion&&) = default;
	/**
	 * @brief 演算子処理を行います。
	 */
	Quaternion& operator=(Quaternion&&) = default;
	Quaternion(const XMFLOAT4& q) : XMFLOAT4(q) {}
	// クォータニオンをベクトルから構築
	Quaternion(const XMVECTOR& v)
	{
		XMStoreFloat4(this, v);
	}
	// 単位クォータニオンを返す
	static const Quaternion Identity;

	// クォータニオンの正規化
	/**
	 * @brief Normalize の処理を行います。
	 */
	void Normalize();

	// クォータニオンの共役を返す
	/**
	 * @brief Conjugate の処理を行います。
	 */
	Quaternion Conjugate() const;

	// クォータニオンの逆数を返す
	/**
	 * @brief Inverse の処理を行います。
	 */
	Quaternion Inverse() const;

	// クォータニオンの成分にアクセス
	/**
	 * @brief 演算子処理を行います。
	 * @return 処理結果を返します。
	 */
	float& operator[](size_t index);

	/**
	 * @brief 演算子処理を行います。
	 * @return 処理結果を返します。
	 */
	float operator[](size_t index) const;

	// クォータニオンの乗算
	/**
	 * @brief 演算子処理を行います。
	 * @return 処理結果を返します。
	 */
	Quaternion operator*(const Quaternion& rhs) const;

	// クォータニオンのイコール比較
	/**
	 * @brief 演算子処理を行います。
	 */
	bool operator==(const Quaternion& rhs) const;
	/**
	 * @brief 演算子処理を行います。
	 * @return 処理結果を返します。
	 */
	bool operator!=(const Quaternion& rhs) const;

	// クォータニオンを XMVECTOR に変換
	/**
	 * @brief ToXMVector の処理を行います。
	 */
	XMVECTOR ToXMVector() const;

	// クォータニオンをオイラー角（度）に変換
	/**
	 * @brief ToEuler の処理を行います。
	 */
	Vector3 ToEuler() const;


	// クォータニオンを前ベクトルに変換
	/**
	 * @brief Forward の処理を行います。
	 */
	Vector3 Forward() const;

	// クォータニオンを右ベクトルに変換
	/**
	 * @brief Right の処理を行います。
	 */
	Vector3 Right() const;

	// クォータニオンを上ベクトルに変換
	/**
	 * @brief Up の処理を行います。
	 */
	Vector3 Up() const;

	// クォータニオンを回転行列に変換
	/**
	 * @brief ToMatrix の処理を行います。
	 */
	XMMATRIX ToMatrix() const;

	// クォータニオン同士のイコール比較
	/**
	 * @brief Equal の処理を行います。
	 */
	static bool Equal(const Quaternion& q1, const Quaternion& q2);

	// クォータニオン同士のノットイコール比較
	/**
	 * @brief NotEqual の処理を行います。
	 */
	static bool NotEqual(const Quaternion& q1, const Quaternion& q2);

	// クォータニオン同士の近似イコール比較
	/**
	 * @brief NearEqual の処理を行います。
	 */
	static bool NearEqual(const Quaternion& q1, const Quaternion& q2, float epsilon = 1e-4f);

	// クォータニオンの正規化
	/**
	 * @brief Normalized の処理を行います。
	 */
	static Quaternion Normalized(const Quaternion& q);

	// オイラー角（度）をクォータニオンに変換
	/**
	 * @brief FromEuler の処理を行います。
	 */
	static Quaternion FromEuler(const Vector3& euler);

	// クォータニオンを回転行列に変換
	/**
	 * @brief LookAt の処理を行います。
	 */
	static Quaternion LookAt(const Vector3& from, const Vector3& to, const Vector3& up = Vector3::Up);

	// クォータニオンを任意軸回りの回転に変換
	/**
	 * @brief RotationAxis の処理を行います。
	 */
	static Quaternion RotationAxis(const Vector3& axis, float angle);

	// クォータニオンを指定軸の回転角に変換
	/**
	 * @brief ToAxisAngle の処理を行います。
	 */
	static float ToAxisAngle(const Vector3& axis, const Quaternion& q);

	// クォータニオン同士の線形補間
	/**
	 * @brief Slerp の処理を行います。
	 */
	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);

	// クォータニオン同士の乗算
	/**
	 * @brief Multiply の処理を行います。
	 */
	static Quaternion Multiply(const Quaternion& q1, const Quaternion& q2);

};