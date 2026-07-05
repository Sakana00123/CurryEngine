#pragma once
#include "Engine/Core/GameObject.h"
#include "Engine/UI/Slider.h"
class Scene;

class GameObjectFactory
{
public:
	/**
	 * @brief 通常のゲームオブジェクトを生成します。
	 * @param name オブジェクト名。
	 * @return 生成された `GameObject*`。
	 */
	static GameObject* Create(Scene* scene, const std::string& name);
	/** @brief UI 用ゲームオブジェクトを生成します。(座標だけの場合)*/
	static GameObject* CreateUIObject(Scene* scene, const std::string& name, GameObject* canvas);
	/** @brief UI 用ゲームオブジェクトを生成します。*/
	static GameObject* CreateUI(Scene* scene, const std::string& name, GameObject* canvas);
	/** @brief キャンバスを生成します。*/
	static GameObject* CreateCanvas(Scene* scene, const std::string& name);
	/** @brief キャンバス引数を解決し、有効なポインタを返します。*/
	static GameObject* ResolveCanvasObject(Scene* scene, GameObject* canvas);
	/** @brief 画像 UI を生成します。*/
	static GameObject* CreateImage(Scene* scene, const std::string& name, GameObject* canvas = nullptr, const wchar_t* sourceImage = nullptr);
	/** @brief ボタン UI を生成します。*/
	static GameObject* CreateButton(Scene* scene, const std::string& name, GameObject* canvas = nullptr, const wchar_t* sourceImage = nullptr);
	/** @brief トグル UI を生成します。*/
	static GameObject* CreateToggle(Scene* scene, const std::string& name, GameObject* canvas = nullptr, const wchar_t* background = L"./Data/Default/UISprite.png", const wchar_t* check = L"./Data/Default/check.png");
	/** @brief スライダー UI を生成します。*/
	static GameObject* CreateSlider(Scene* scene, const std::string& name, GameObject* canvas = nullptr, float min = 0.f, float max = 1.f, float defaultValue = 0.f,
		Slider::Direction direction = Slider::Direction::LeftToRight, const wchar_t* handleImage = L"./Data/Default/UISprite.png", const wchar_t* backGroundImage = L"./Data/Default/Background.png");
	/** @brief テキスト UI を生成します。*/
	static GameObject* CreateText(Scene* scene, const std::string& name, GameObject* canvas = nullptr, const std::wstring& text = L"Text", const std::string& fontFilePath = "./Assets/Fonts/madoufmg.fnt",
		const char* customPsName = nullptr, const char* customVsName = nullptr, size_t maxElements = 256);
	/** @brief 入力フィールド UI を生成します。*/
	static GameObject* CreateInputField(Scene* scene, const std::string& name, GameObject* canvas = nullptr, const std::string& fontFilePath = "./Assets/Fonts/madoufmg.fnt", const wchar_t* backGroundImage = L"./Data/Default/waku.png",
		const char* customPsName = nullptr, const char* customVsName = nullptr, size_t maxElements = 256);
	/** @brief スクロールビュー UI を生成します。*/
	static GameObject* CreateScrollView(Scene* scene, const std::string& name, GameObject* canvas = nullptr);
	/** @brief UI イベントシステムを生成します。*/
	static GameObject* CreateEventSystem(Scene* scene, const std::string& name);
	/** @brief 立方体を生成します。*/
	static GameObject* CreateCube(Scene* scene, const std::string& name);
	/** @brief 円柱を生成します。*/
	static GameObject* CreateCylinder(Scene* scene, const std::string& name, int segment = 20);
	/** @brief 球体を生成します。*/
	static GameObject* CreateSphere(Scene* scene, const std::string& name, int segment = 20);
	/** @brief モデルを読み込み生成します。*/
	static GameObject* CreateModel(Scene* scene, const std::string& name, const std::u8string& filePath, bool staticBatching = false);
	/** @brief オーディオソースを生成します。*/
	static GameObject* CreateAudioSource(Scene* scene, const std::string& name, const wchar_t* filePath = L"Data/Sounds/Demo.wav");
	/** @brief カメラを生成します。*/
	static GameObject* CreateCamera(Scene* scene, const std::string& name);
	/** @brief ディレクショナルライトを生成します。*/
	static GameObject* CreateDirectionalLight(Scene* scene, const std::string& name);
	/** @brief ポイントライトを生成します。*/
	static GameObject* CreatePointLight(Scene* scene, const std::string& name);
	/** @brief スポットライトを生成します。*/
	static GameObject* CreateSpotLight(Scene* scene, const std::string& name);
};
