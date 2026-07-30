#include "pch.h"
#include "DebugRenderer.h"
#include "Engine/Rendering/Pipeline/RenderContext.h"
//#include "Engine/Core/Math/Matrix4x4.h"
#include "Engine/Resources/Shader.h"
#include "Engine/Rendering/Pipeline/Graphics.h"
#include "Engine/Core/EnginePaths.h"

void DebugRenderer::Initialize()
{
	// デバッグ描画の初期化処理を実装
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	auto device = Graphics::GetDevice();
	std::string dir = EnginePaths::ShadersDataDir;
	CreateVertexShaderFromCSO(device, (dir + "DebugRendererVS.cso").c_str(), vertexShader.ReleaseAndGetAddressOf(),
		inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
	CreatePixelShaderFromCSO(device, (dir + "DebugRendererPS.cso").c_str(), pixelShader.ReleaseAndGetAddressOf());

	// 頂点バッファの作成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(Vertex) * VertexCapacity;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, vertexBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

	// 定数バッファの作成
	bufferDesc.ByteWidth = sizeof(ConstantBufferData);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));
}

void DebugRenderer::Finalize()
{
	// デバッグ描画の終了処理を実装
}


void DebugRenderer::DrawAll(RenderContext* rtx, D3D11_PRIMITIVE_TOPOLOGY topology)
{
	// デバッグ描画の実行処理を実装
	auto dc = rtx->immediateContext;
	auto& view = rtx->view;
	auto& projection = rtx->projection;

	// シェーダー設定
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
	dc->IASetInputLayout(inputLayout.Get());

	// 定数バッファ設定
	dc->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	// ビュープロジェクション行列作成
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&projection);
	DirectX::XMMATRIX VP = V * P;

	// 定数バッファ更新
	ConstantBufferData data;
	DirectX::XMStoreFloat4x4(&data.viewProjection, VP);
	dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);

	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	dc->IASetPrimitiveTopology(topology);
	dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// 描画
	UINT totalVertexCount = static_cast<UINT>(vertices.size());
	UINT start = 0;
	UINT count = (totalVertexCount < VertexCapacity) ? totalVertexCount : VertexCapacity;

	while (start < totalVertexCount)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		_ASSERT_EXPR(SUCCEEDED(hr), HrTrace(hr));

		memcpy(mappedSubresource.pData, &vertices[start], sizeof(Vertex) * count);

		dc->Unmap(vertexBuffer.Get(), 0);

		dc->Draw(count, 0);

		start += count;
		if ((start + count) > totalVertexCount)
		{
			count = totalVertexCount - start;
		}
	}

	vertices.clear();

}

void DebugRenderer::AddVertex(const Vector3& position, const Color& color)
{
	// 頂点を追加する処理を実装
	if (vertices.size() < VertexCapacity)
	{
		vertices.push_back({ position, color });
	}
}


void DebugRenderer::DrawLine(const Vector3& start, const Vector3& end, const Color& color)
{
	// 線分を描画する処理を実装
	AddVertex(start, color);
	AddVertex(end, color);
}

void DebugRenderer::DrawBox(const Vector3& center, const Quaternion& rotation, const Vector3& size, const Color& color)
{
	// 立方体を描画する処理を実装
	Vector3 halfSize = size * 0.5f;
	Vector3 vertices[] = {
		Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(halfSize.x, halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, -halfSize.y, halfSize.z),
		Vector3(halfSize.x, -halfSize.y, halfSize.z),
		Vector3(halfSize.x, halfSize.y, halfSize.z),
		Vector3(-halfSize.x, halfSize.y, halfSize.z)
	};
	DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&rotation)));
	for (int i = 0; i < _countof(vertices); ++i)
	{
		// 回転を適用
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&vertices[i]));
		v = DirectX::XMVector3Transform(v, rotMatrix);
		DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&vertices[i]), v);

		// 中心位置を加算
		vertices[i] += center;
	}

	int edges[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0},
		{4, 5}, {5, 6}, {6, 7}, {7, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	};
	for (int i = 0; i < 12; ++i)
	{
		DrawLine(vertices[edges[i][0]], vertices[edges[i][1]], color);
	}
}

void DebugRenderer::DrawSphere(const Vector3& center, float radius, const Color& color, int segments)
{
	// 球を描画する処理を実装
	for (int i = 0; i < segments; ++i)
	{
		float theta1 = (float)i / segments * DirectX::XM_2PI;
		float theta2 = (float)(i + 1) / segments * DirectX::XM_2PI;
		Vector3 p1 = center + Vector3(radius * cosf(theta1), 0, radius * sinf(theta1));
		Vector3 p2 = center + Vector3(radius * cosf(theta2), 0, radius * sinf(theta2));
		DrawLine(p1, p2, color);
		p1 = center + Vector3(0, radius * cosf(theta1), radius * sinf(theta1));
		p2 = center + Vector3(0, radius * cosf(theta2), radius * sinf(theta2));
		DrawLine(p1, p2, color);
		p1 = center + Vector3(radius * cosf(theta1), radius * sinf(theta1), 0);
		p2 = center + Vector3(radius * cosf(theta2), radius * sinf(theta2), 0);
		DrawLine(p1, p2, color);
	}
}

void DebugRenderer::DrawHemisphere(const Vector3& center, const Vector3& direction, float radius, const Color& color, bool drawBottom, int segments)
{
	if (direction.LengthSq() == 0.0f)
	{
		// 方向ベクトルがゼロの場合は描画しない
		LOG_ERROR("DrawHemisphere: direction vector is zero. Cannot determine hemisphere orientation.");
		return;
	}

	using namespace DirectX;
	XMVECTOR dir = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&direction));
	dir = XMVector3Normalize(dir);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX rotation;
	float dot = XMVectorGetX(XMVector3Dot(up, dir));

	// upとdirがほぼ平行な場合（外積がゼロにならないようにする）
	if (dot > 0.999f)
	{
		rotation = XMMatrixIdentity();
	}
	else if (dot < -0.999f)
	{
		rotation = XMMatrixRotationX(XM_PI);
	}
	else
	{
		XMVECTOR axis = XMVector3Normalize(XMVector3Cross(up, dir));
		float angle = acosf(dot);
		rotation = XMMatrixRotationAxis(axis, angle);
	}

	// 半球の頂点を計算して描画
	for (int i = 0; i < segments; ++i)
	{
		// 半球なので、天頂角(theta)は 0 から PI/2 の範囲にする
		float theta1 = (float)i / segments * DirectX::XM_PIDIV2;
		float theta2 = (float)(i + 1) / segments * DirectX::XM_PIDIV2;
		for (int j = 0; j < segments; ++j)
		{
			float phi1 = (float)j / segments * DirectX::XM_2PI;
			float phi2 = (float)(j + 1) / segments * DirectX::XM_2PI;

			// ローカル座標で計算 (まだcenterを足さない)
			Vector3 localP1(radius * sinf(theta1) * cosf(phi1), radius * cosf(theta1), radius * sinf(theta1) * sinf(phi1));
			Vector3 localP2(radius * sinf(theta1) * cosf(phi2), radius * cosf(theta1), radius * sinf(theta1) * sinf(phi2));
			Vector3 localP3(radius * sinf(theta2) * cosf(phi2), radius * cosf(theta2), radius * sinf(theta2) * sinf(phi2));
			Vector3 localP4(radius * sinf(theta2) * cosf(phi1), radius * cosf(theta2), radius * sinf(theta2) * sinf(phi1));

			// 回転を適用
			XMVECTOR P1 = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localP1)), rotation);
			XMVECTOR P2 = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localP2)), rotation);
			XMVECTOR P3 = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localP3)), rotation);
			XMVECTOR P4 = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localP4)), rotation);

			// 回転させた後にcenter（ワールド座標）を足す
			Vector3 wp1 = center + Vector3(XMVectorGetX(P1), XMVectorGetY(P1), XMVectorGetZ(P1));
			Vector3 wp2 = center + Vector3(XMVectorGetX(P2), XMVectorGetY(P2), XMVectorGetZ(P2));
			Vector3 wp3 = center + Vector3(XMVectorGetX(P3), XMVectorGetY(P3), XMVectorGetZ(P3));
			Vector3 wp4 = center + Vector3(XMVectorGetX(P4), XMVectorGetY(P4), XMVectorGetZ(P4));

			// 線分を描画
			DrawLine(wp1, wp2, color);
			DrawLine(wp2, wp3, color);
			DrawLine(wp3, wp4, color);
			DrawLine(wp4, wp1, color);
		}
	}

	// 底面を描画する場合
	if (drawBottom)
	{
		for (int j = 0; j < segments; ++j)
		{
			float phi1 = (float)j / segments * DirectX::XM_2PI;
			float phi2 = (float)(j + 1) / segments * DirectX::XM_2PI;

			// ローカル座標で計算
			Vector3 localP1(radius * cosf(phi1), 0.0f, radius * sinf(phi1));
			Vector3 localP2(radius * cosf(phi2), 0.0f, radius * sinf(phi2));

			// 回転を適用
			XMVECTOR P1 = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localP1)), rotation);
			XMVECTOR P2 = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localP2)), rotation);

			// centerを足す
			Vector3 wp1 = center + Vector3(XMVectorGetX(P1), XMVectorGetY(P1), XMVectorGetZ(P1));
			Vector3 wp2 = center + Vector3(XMVectorGetX(P2), XMVectorGetY(P2), XMVectorGetZ(P2));

			// 線分を描画
			DrawLine(wp1, wp2, color);
		}
	}
}

void DebugRenderer::DrawCylinder(const Vector3& start, const Vector3& end, float radius, const Color& color, bool drawTopBottom, int segments)
{
	if (start == end)
	{
		LOG_ERROR("DrawCylinder: start and end points are the same. Cannot draw a cylinder.");
		return;
	}

	using namespace DirectX;

	// 方向ベクトルから、ワールド空間での円柱の向きを計算
	Vector3 direction = (end - start).Normalize();
	XMVECTOR dir = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&direction));
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX rotation;
	float dot = XMVectorGetX(XMVector3Dot(up, dir));

	// upとdirがほぼ平行な場合（外積がゼロにならないようにする）
	if (dot > 0.999f)
	{
		rotation = XMMatrixIdentity();
	}
	else if (dot < -0.999f)
	{
		rotation = XMMatrixRotationX(XM_PI);
	}
	else
	{
		XMVECTOR axis = XMVector3Normalize(XMVector3Cross(up, dir));
		float angle = acosf(dot);
		rotation = XMMatrixRotationAxis(axis, angle);
	}

	// 頂点を計算して描画
	float thetaStep = XM_2PI / segments;
	for (int i = 0; i < segments; ++i)
	{
		float theta1 = i * thetaStep;
		float theta2 = (i + 1) * thetaStep;

		// XZ平面での円周上のローカル座標
		Vector3 localP1(radius * cosf(theta1), 0.0f, radius * sinf(theta1));
		Vector3 localP2(radius * cosf(theta2), 0.0f, radius * sinf(theta2));

		XMVECTOR vLocalP1 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localP1));
		XMVECTOR vLocalP2 = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localP2));

		// 回転を適用
		XMVECTOR vRotP1 = XMVector3TransformNormal(vLocalP1, rotation);
		XMVECTOR vRotP2 = XMVector3TransformNormal(vLocalP2, rotation);

		Vector3 rotP1, rotP2;
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&rotP1), vRotP1);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&rotP2), vRotP2);

		// startとendに足し合わせてワールド座標の点を求める
		Vector3 p1 = start + rotP1;
		Vector3 p2 = start + rotP2;
		Vector3 p3 = end + rotP2;
		Vector3 p4 = end + rotP1;

		DrawLine(p1, p2, color);
		DrawLine(p2, p3, color);
		DrawLine(p3, p4, color);
		DrawLine(p4, p1, color);

		if (drawTopBottom)
		{
			DrawLine(start, p1, color);
			DrawLine(start, p2, color);
			DrawLine(end, p3, color);
			DrawLine(end, p4, color);
		}
	}
}

void DebugRenderer::DrawCapsule(const Vector3& start, const Vector3& end, float radius, const Color& color, int segments)
{
	// カプセルを描画する処理を実装(半球、円柱、半球の順に結合部分を省いて描画)
	if (start == end)
	{
		DrawSphere(start, radius, color, segments);
		return;
	}

	DrawHemisphere(start, (start - end).Normalize(), radius, color, false, segments);
	DrawCylinder(start, end, radius, color, false, segments);
	DrawHemisphere(end, (end - start).Normalize(), radius, color, false, segments);
}

void DebugRenderer::DrawPlane(const Vector3& center, const Vector3& normal, float size, const Color& color)
{
	// 平面を描画する処理を実装
	Vector3 up = Vector3::Up;
	if (fabs(normal.Dot(up)) > 0.99f)
	{
		up = Vector3::Right;
	}
	Vector3 right = normal.Cross(up).Normalize();
	up = right.Cross(normal).Normalize();
	Vector3 halfSize = size * 0.5f;
	Vector3 corners[4] = {
		center + (-right - up) * halfSize,
		center + (right - up) * halfSize,
		center + (right + up) * halfSize,
		center + (-right + up) * halfSize
	};
	DrawLine(corners[0], corners[1], color);
	DrawLine(corners[1], corners[2], color);
	DrawLine(corners[2], corners[3], color);
	DrawLine(corners[3], corners[0], color);
}

void DebugRenderer::DrawGrid(const Vector3& center, float size, int divisions, const Color& color)
{
	// グリッドを描画する処理を実装
	float halfSize = size * 0.5f;
	float step = size / divisions;
	for (int i = 0; i <= divisions; ++i)
	{
		float offset = -halfSize + i * step;
		// X軸方向の線
		DrawLine({ center.x + offset, center.y, center.z - halfSize }, { center.x + offset, center.y, center.z + halfSize }, color);
		// Z軸方向の線
		DrawLine({ center.x - halfSize, center.y, center.z + offset }, { center.x + halfSize, center.y, center.z + offset }, color);
	}
}
