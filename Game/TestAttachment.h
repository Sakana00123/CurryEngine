#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Core/Transform.h"
#include "Pin.h"

class TestAttachment : public Component
{
	C_REFLECT(TestAttachment)
public:
	TestAttachment() = default;
	~TestAttachment() = default;

public:

	//Component のライフサイクルイベントを必要に応じてオーバーライドして実装します。
	void Start() override;
	void Update(float deltaTime) override;


	// ピンにオブジェクトをアタッチする関数
	bool AttachToPin(std::string& path, Pin* hitPin) const;

	// ピンにオブジェクトをアタッチする関数
	bool AttachToPin(GameObject* prefab, Pin* hitPin) const;

private:

	// オブジェクト参照プロパティを定義する場合は、C_PROPERTY() マクロの引数に ObjectReference 属性を指定します。引数には参照先の型名を文字列で指定します。
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GameObject"))
	ObjectId pinObj; 

	C_PROPERTY()
	std::string prefabPath; // プレハブのファイルパス


};
