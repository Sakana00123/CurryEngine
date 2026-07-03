#include "pch.h"
#include "GameObjectFactory.h"

#include "Engine/Scenes/Scene.h"

#include "Engine/UI/Canvas.h"
#include "Engine/UI/Image.h"
#include "Engine/UI/Button.h"
#include "Engine/UI/Toggle.h"
//#include "2D/UI/Slider.h"
#include "Engine/UI/Text.h"
#include "Engine/UI/InputField.h"
#include "Engine/UI/ScrollView.h"
#include "Engine/UI/GraphicRaycaster.h"
#include "Engine/Events/EventSystem.h"
#include "Engine/Events/InputModule.h"
#include "Engine/Rendering/Renderers/GltfModelRenderer.h"
#include "Engine/Rendering/Renderers/PrimitiveRenderer.h"
#include "Engine/Physics/Collider.h"
#include "Engine/Physics/BoxCollider.h"
#include "Engine/Physics/SphereCollider.h"
#include "Engine/Physics/CapsuleCollider.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/AudioSource.h"
#include "Engine/Rendering/Camera/CameraComponent.h"
#include "Engine/Rendering/Lights/DirectionalLightComponent.h"
#include "Engine/Rendering/Lights/PointLightComponent.h"
#include "Engine/Rendering/Lights/SpotLightComponent.h"
#include "Engine/Utils/RectTransformUtils.h"
#include "Engine/UI/Mask.h"

GameObject* GameObjectFactory::Create(Scene* scene, const std::string& name) {
	std::shared_ptr<GameObject> object = std::make_shared<GameObject>();
	object->Create(name);
	scene->objectManager->Register(object);
	return object.get();
}
GameObject* GameObjectFactory::CreateUIObject(Scene* scene, const std::string& name, GameObject* canvas) {
	GameObject* obj = CreateUI(scene, name, canvas);
	obj->AddComponent<UIComponent>();
	return obj;
}
GameObject* GameObjectFactory::CreateUI(Scene* scene, const std::string& name, GameObject* canvas) {
	GameObject* obj = Create(scene, name);
	obj->SetParent(ResolveCanvasObject(scene, canvas));
	for (const auto& comp : obj->_components) {
		if (comp)
		{
			comp->OnDestroy();
			comp->SetEnabled(false);
		}
	}
	obj->_components.clear();
	obj->transform = obj->AddComponent<RectTransform>();
	return obj;
}
GameObject* GameObjectFactory::CreateCanvas(Scene* scene, const std::string& name) {
	GameObject* obj = Create(scene, name);
	obj->AddComponent<Canvas>();
	obj->AddComponent<GraphicRaycaster>();
	RectTransform* rect = obj->GetComponent<RectTransform>();
	rect->SetPivot({ 0,0 });

	if (!scene->objectManager->FindInObjects("EventSystem")) {
		CreateEventSystem(scene, "EventSystem");
	}
	return obj;
}
//有効なポインタとして返す
GameObject* GameObjectFactory::ResolveCanvasObject(Scene* scene, GameObject* canvas) {
	if (!canvas) {
		if (GameObject* obj = scene->objectManager->FindInObjects("Canvas")) {
			canvas = obj;
		}
		else {
			canvas = CreateCanvas(scene, "Canvas");
		}
	}
	return canvas;
}
GameObject* GameObjectFactory::CreateImage(Scene* scene, const std::string& name, GameObject* canvas, const wchar_t* sourceImage) {
	GameObject* obj = CreateUI(scene, name, canvas);
	Image* image = obj->AddComponent<Image>();
	if (sourceImage) {
		image->SetSource(sourceImage, true);
	}
	return obj;
}
GameObject* GameObjectFactory::CreateButton(Scene* scene, const std::string& name, GameObject* canvas, const wchar_t* sourceImage) {
	GameObject* obj = CreateImage(scene, name, canvas, sourceImage);
	Button* button = obj->AddComponent<Button>();
	Image* image = obj->GetComponent<Image>();
	button->imageReference = image->GetId();
	image->GetRectTransform()->SetSize({ 300, 150 });

	return obj;
}
GameObject* GameObjectFactory::CreateToggle(Scene* scene, const std::string& name, GameObject* canvas, const wchar_t* background, const wchar_t* check) {
	GameObject* obj = CreateUI(scene, name, canvas);
	Toggle* toggle = obj->AddComponent<Toggle>();
	GameObject* backGround = CreateImage(scene, "Background", canvas, background);
	Image* image = backGround->GetComponent<Image>();
	toggle->imageReference = image->GetId();
	image->GetRectTransform()->SetAnchorMin({ 0,0 });
	image->GetRectTransform()->SetAnchorMax({ 1,1 });
	image->GetRectTransform()->SetPivot({ 0.f,0.f });
	backGround->SetParent(obj);

	GameObject* checkObj = CreateImage(scene, "Checkmark", canvas, check);
	checkObj->SetParent(backGround);
	Image* checkImage = checkObj->GetComponent<Image>();
	toggle->checkMarkReference = checkImage->GetId();
	checkImage->GetRectTransform()->SetAnchorMin({ 0,0 });
	checkImage->GetRectTransform()->SetAnchorMax({ 1,1 });
	checkImage->GetRectTransform()->SetPivot({ 0.f,0.f });
	checkImage->color = Color::Black;
	return obj;
}
GameObject* GameObjectFactory::CreateSlider(Scene* scene, const std::string& name, GameObject* canvas, float min, float max, float defaultValue,
	Slider::Direction direction, const wchar_t* handleImage, const wchar_t* backGroundImage) {
	GameObject* obj = CreateUI(scene, name, canvas);
	Slider* slider = obj->AddComponent<Slider>();
	slider->minValue = min;
	slider->maxValue = max;
	slider->SetValue(defaultValue);
	slider->SetDirection(direction);
	GameObject* back = CreateImage(scene, "Background", canvas, backGroundImage);
	GameObject* fillArea = CreateUI(scene, "Fill Area", canvas);
	GameObject* fill = CreateImage(scene, "Fill", canvas, backGroundImage);
	GameObject* handleArea = CreateUI(scene, "Handle Slide Area", canvas);
	GameObject* handle = CreateImage(scene, "Handle", canvas, handleImage);

	RectTransform* fillRect = fill->GetComponent<RectTransform>();
	RectTransform* fillAreaRect = fillArea->AddComponent<UIComponent>()->GetRectTransform();

	RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
		fillAreaRect,
		{ 0.f, 0.25f },
		{ 1.f, 0.75f },
		fillAreaRect->GetPivot()
	);

	fillRect->pivot = { 0,0.5f };
	//fillAreaRect->SetLeft(5.f);
	//fillAreaRect->SetRight(15.f);

	//fillRect->SetLeft(-5.f);
	//fillRect->SetRight(-5.f);

	RectTransform* handleRect = handle->GetComponent<RectTransform>();
	RectTransform* handleAreaRect = handleArea->AddComponent<UIComponent>()->GetRectTransform();

	handleAreaRect->SetAnchorMin({ 0,0.5f });
	handleAreaRect->SetAnchorMax({ 1,0.5f });
	handleAreaRect->SetAnchoredPosition({ 0,0 });
	handleAreaRect->SetLeft(10.f);
	handleAreaRect->SetRight(-10.f);
	//handleRect->SetAnchorMin({ 0.f, 0.5f });
	//handleRect->SetAnchorMax({ 0.f, 0.5f });

	//slider->image = back->GetComponent<Image>();
	Image* backImage = back->GetComponent<Image>();
	slider->imageReference = backImage->GetId();
	slider->fillRect = fillRect;
	slider->handleRect = handleRect;
	slider->GetRectTransform()->size = { 160,20 };
	back->SetParent(obj);
	fillArea->SetParent(obj);
	fill->SetParent(fillArea);
	handleArea->SetParent(obj);
	handle->SetParent(handleArea);

	_ASSERT_EXPR(slider->GetImage() != nullptr, L"Slider requires an Image component for the background.");
	slider->SetRectTransform(slider->GetImage()->GetRectTransform());
	slider->GetRectTransform()->SetAnchorMin({ 0.f, 0.25f });
	slider->GetRectTransform()->SetAnchorMax({ 1.f, 0.75f });
	slider->GetRectTransform()->SetPivot({ 0.f, 0.5f });
	slider->SetValue(defaultValue);
	return obj;
}
GameObject* GameObjectFactory::CreateText(Scene* scene, const std::string& name, GameObject* canvas, const std::wstring& text, const std::string& fontFilePath,
	const char* customPsName, const char* customVsName, size_t maxElements) {
	GameObject* obj = CreateUI(scene, name, canvas);
	Text* textComponent = obj->AddComponent<Text>();
	RectTransform* rect = obj->GetComponent<RectTransform>();
	if (!rect) {
		rect = obj->AddComponent<RectTransform>();
		obj->RemoveComponent<Transform>();
		obj->transform = rect;
	}
	textComponent->SetRectTransform(rect);
	rect->SetSize({ 200,200 });
	textComponent->Setup(fontFilePath, customPsName, customVsName);
	textComponent->text = text;
	textComponent->isRaycastTarget = false;
	textComponent->alignment = Text::Alignment::MiddleCenter;
	textComponent->color = Color::Black;

	return obj;
}
GameObject* GameObjectFactory::CreateInputField(Scene* scene, const std::string& name, GameObject* canvas, const std::string& fontFilePath, const wchar_t* backGroundImage,
	const char* customPsName, const char* customVsName, size_t maxElements) {
	ID3D11Device* device = Graphics::GetDevice();
	GameObject* obj = CreateUI(scene, name, canvas);
	InputField* inputField = obj->AddComponent<InputField>();
	RectTransform* rect = obj->GetComponent<RectTransform>();
	rect->SetSize({ 550,100 });
	inputField->SetRectTransform(rect);
	Image* image = obj->AddComponent<Image>();
	image->SetSource(backGroundImage, true);
	inputField->imageReference = image->GetId();

	GameObject* textObj = CreateText(scene, "Text", canvas);
	Text* textComponent = textObj->GetComponent<Text>();
	textComponent->SetAlignment(Text::Alignment::TopLeft);
	inputField->textComponentRef = textComponent->GetId();
	RectTransform* textRect = textComponent->GetRectTransform();
	textRect->SetAnchorMin({ 0,0 });
	textRect->SetAnchorMax({ 0,0 });
	textRect->SetPivot({ 0,0 });
	textRect->SetSize({ 200,200 });
	textRect->SetAnchoredPosition({ 50,20 });
	textObj->SetParent(obj);
	inputField->GetRectTransform()->SetAnchoredPosition({});
	return obj;
}
GameObject* GameObjectFactory::CreateScrollView(Scene* scene, const std::string& name, GameObject* canvas) {
	GameObject* obj = CreateUI(scene, name, canvas);
	obj->AddComponent<ScrollView>();
	obj->AddComponent<Mask>();

	// スクロールビューの構造を作成
	GameObject* content = CreateUIObject(scene, "Content", canvas);
	RectTransform* contentRect = content->GetComponent<RectTransform>();
	contentRect->SetAnchorMin({ 0,0 });
	contentRect->SetAnchorMax({ 0,0 });
	contentRect->SetPivot({ 0,0 });
	contentRect->SetSize({ 200,200 });

	content->SetParent(obj);

	// ScrollView コンポーネントに Content を設定
	ScrollView* scrollView = obj->GetComponent<ScrollView>();
	scrollView->contentRef = content->GetId();

	return obj;
}
GameObject* GameObjectFactory::CreateEventSystem(Scene* scene, const std::string& name) {
	GameObject* obj = Create(scene, name);
	obj->AddComponent<InputModule>();
	return obj;
}
GameObject* GameObjectFactory::CreateCube(Scene* scene, const std::string& name) {
	GameObject* obj = Create(scene, name);
	obj->AddComponent<PrimitiveRenderer>()->SetShape(PrimitiveRenderer::Shape::Cube);
	obj->AddComponent<BoxCollider>()->autoFit = true;
	return obj;
}
GameObject* GameObjectFactory::CreateCylinder(Scene* scene, const std::string& name, int segment) {
	GameObject* obj = Create(scene, name);
	obj->AddComponent<PrimitiveRenderer>()->SetShape(PrimitiveRenderer::Shape::Cylinder);
	obj->AddComponent<CapsuleCollider>()->autoFit = true;
	return obj;
}
GameObject* GameObjectFactory::CreateSphere(Scene* scene, const std::string& name, int segment) {
	GameObject* obj = Create(scene, name);
	obj->AddComponent<PrimitiveRenderer>()->SetShape(PrimitiveRenderer::Shape::Sphere);
	obj->AddComponent<SphereCollider>()->autoFit = true;
	return obj;
}
GameObject* GameObjectFactory::CreateModel(Scene* scene, const std::string& name, const std::string& filePath, bool staticBatching) {
	GameObject* obj = Create(scene, name);
	obj->AddComponent<GltfModelRenderer>()->LoadModel(Graphics::GetDevice(), filePath, staticBatching);
	return obj;
}
GameObject* GameObjectFactory::CreateAudioSource(Scene* scene, const std::string& name, const wchar_t* filePath) {
	GameObject* obj = Create(scene, name);
	AudioSource* audioSource = obj->AddComponent<AudioSource>();
	audioSource->SetSource(filePath);
	return obj;
}

GameObject* GameObjectFactory::CreateCamera(Scene* scene, const std::string& name)
{
	GameObject* obj = Create(scene, name);
	obj->AddComponent<CameraComponent>();
	return obj;
}

GameObject* GameObjectFactory::CreateDirectionalLight(Scene* scene, const std::string& name)
{
	GameObject* obj = Create(scene, name);
	obj->AddComponent<DirectionalLightComponent>();
	return obj;
}

GameObject* GameObjectFactory::CreatePointLight(Scene* scene, const std::string& name)
{
	GameObject* obj = Create(scene, name);
	obj->AddComponent<PointLightComponent>();
	return obj;
}

GameObject* GameObjectFactory::CreateSpotLight(Scene* scene, const std::string& name)
{
	GameObject* obj = Create(scene, name);
	obj->AddComponent<SpotLightComponent>();
	return obj;
}
