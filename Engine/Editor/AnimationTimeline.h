#pragma once
#ifdef USE_IMGUI
#include <imgui.h>
#include "imgui_internal.h"
#include <ImSequencer.h>
#endif // USE_IMGUI
#include <vector>
#include <memory>
#include "Archive/gltf_model.h"

#ifdef USE_IMGUI
struct ItemData
{
    int start = 0;     // 出現時間（フレーム or ミリ秒）
    int end = 0;       // ほぼ start と同じで OK（1フレーム長）
    int type = 0;      // 敵種
    unsigned int color = 0xFFFFFFFF;
};

class AnimationTimeline : public ImSequencer::SequenceInterface
{
public:
	static constexpr int TYPE_SOUND = 0;
	static constexpr int TYPE_EFFECT = 1;
	static constexpr int TYPE_ENABLE_HIT = 2;
	static constexpr int TYPE_COUNT = 3;

    std::vector<ItemData> items;
	int currentFrame = 0;
	int firstFrame = 0;
	int frameCount = 6000;

public:

    // ==== 必須 =========================================================
    int GetFrameMin() const override { return firstFrame; }
    int GetFrameMax() const override { return frameCount; } // 最大フレーム（適当に）

    int GetItemCount() const override {
        return (int)items.size();
    }

    // index のアイテムを返す
    void Get(
        int index,
        int** start,
        int** end,
        int* type,
        unsigned int* color
    ) override
    {
        auto& it = items[index];
        if (start) *start = &it.start;
        if (end)   *end = &it.end;
        if (type)  *type = it.type;
        if (color) *color = it.color;
    }

    // ==== 種類 / 追加ボタン =============================================
    int GetItemTypeCount() const override {
        return TYPE_COUNT;
    }

    const char* GetItemTypeName(int typeIndex) const override {
        switch (typeIndex) {
        case TYPE_SOUND: return "Sound";
        case TYPE_EFFECT: return "Effect";
		case TYPE_ENABLE_HIT: return "EnableHit";
        }
        return "Unknown";
    }

    // 新規追加
    void Add(int type) override {
        ItemData item;
        item.start = 0;
        item.end = 10;
        item.type = type;

        // 色分けしたいならここ
        static unsigned int typeColor[TYPE_COUNT] = {
            0xFF44AAFF, // Sound
            0xFFFFAA44, // Effect
			0xFFAA44FF, // EnableHit
        };
        item.color = typeColor[type % TYPE_COUNT];

        items.push_back(item);
    }

    // 削除
    void Del(int index) override {
        if (index < 0 || index >= items.size()) return;
        items.erase(items.begin() + index);
    }

    // 複製
    void Duplicate(int index) override {
        if (index < 0 || index >= items.size()) return;
        items.push_back(items[index]);
    }

    // ==== アイテム名 ====================================================
    const char* GetItemLabel(int index) const override {
        static char buf[64];
        sprintf_s(buf, "Item %d (Type %d)", index, items[index].type);
        return buf;
    }

    // ==== カスタム描画（レーン分けしたい場合） ============================
    void CustomDraw(
        int index,
        ImDrawList* draw_list,
        const ImRect& rc,
        const ImRect& legendRect,
        const ImRect& clippingRect,
        const ImRect& legendClippingRect
    ) override
    {
        //// lane パラメータによる縦位置ずらし 等を行える
        //const auto& item = items[index];

        //// レーン高さ設定
        //const float laneHeight = 20.0f;        // 1レーンの高さ
        //const float laneOffset = item.lane * laneHeight;

        //// rc（本来バーの位置）にオフセットを追加
        //ImRect laneRc = rc;
        //laneRc.Min.y += laneOffset;
        //laneRc.Max.y += laneOffset;

        //// クリップを設定
        //draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);

        //// バーを描画
        //draw_list->AddRectFilled(
        //    laneRc.Min,
        //    laneRc.Max,
        //    IM_COL32(80, 160, 255, 255) // 適当な色（type/colorに応じて変えてもいい）
        //);

        //draw_list->AddRect(
        //    laneRc.Min,
        //    laneRc.Max,
        //    IM_COL32(255, 255, 255, 255)
        //);

        //draw_list->PopClipRect();
    }

    size_t GetCustomHeight(int index) override {
        return 0; // デフォルトの高さで良い場合
    }
};
#endif // USE_IMGUI


// アニメーションタイムラインエディタ
class AnimationTimelineEditor
{
	static inline bool isOpen = false;
	static inline std::unique_ptr<GltfModel> gltfModel;
	static inline std::string animationName;
	static inline float animationDuration = 0.0f;
public:
    static void Show()
    {
        isOpen = true;
	}

    static void Close()
    {
		isOpen = false;
	}

    static void DrawGUI()
    {
#ifdef USE_IMGUI
        static AnimationTimeline sequence;
        static int currentFrame = 0;
        static bool expanded = true;
        static int selectedEntry = -1;
        static int firstFrame = 0;

		static bool isFirst = true;
        if (isFirst)
        {
            // 初回のみ、サンプルデータを追加しておく
            for (int i = 0; i < 3; i++)
            {
                ItemData item;
                item.start = 0;
                item.end = item.start + 10;
                item.type = i % 3;
                static unsigned int typeColor[3] = {
                    0xFF44AAFF, // Sound
                    0xFFFFAA44, // Effect
					0xFFAA44FF, // EnableHit
                };
                item.color = typeColor[item.type];
                sequence.items.push_back(item);
            }
			isFirst = false;
        }

        if (isOpen)
        {
            ImGui::Begin("Animation Timeline", &isOpen);

			// Load / Save ボタン
            {
                if (ImGui::Button("Load"))
                {
                    char path[260] = {};
					DialogResult result = Dialog::OpenFileName(path, sizeof(path), "GLTF Files\0*.gltf;*.glb\0All Files\0*.*\0", "Load GLTF Model");
                    if (result == DialogResult::OK)
                    {
                        gltfModel = std::make_unique<GltfModel>(Graphics::GetDevice(), path);

						std::filesystem::path filepathObj(path);
						gltfModel->animationIndex = 0; // とりあえず最初のアニメーションを選択

                        if (gltfModel->animations.size() == 0)
                        {
                            animationName = "No Animation";
                            animationDuration = 0.0f;
						}

                        if (gltfModel->animationIndex < 0 || gltfModel->animationIndex >= gltfModel->animations.size())
                        {
                            gltfModel->animationIndex = 0; // 安全策
						}

                        std::filesystem::path animeventPath = filepathObj.replace_extension(".animevent");
                        // .animeventが存在してたらそれもロード
                        if (std::filesystem::exists(animeventPath))
                        {
                            //json j;
                            //JsonFileHandler::LoadJsonFromFile(j, animeventPath.string());
                            //for (const auto& data : j)
                            //{
                            //    (data.size())
                            //}
                        }
                        else
                        {
                            auto& data = gltfModel->animations.at(gltfModel->animationIndex);
                            animationName = data.name;
                            animationDuration = data.duration;
                        }
                    }
                }
				
                ImGui::SameLine();


                if (ImGui::Button("Save"))
                {

                }
            }


            // シーケンサー描画
            ImSequencer::Sequencer(
                &sequence,
                &currentFrame,
                &expanded,
                &selectedEntry,
                &firstFrame,
                ImSequencer::SEQUENCER_EDIT_ALL | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE
            );


            ImGui::End();
        }
#endif // USE_IMGUI
    }
};