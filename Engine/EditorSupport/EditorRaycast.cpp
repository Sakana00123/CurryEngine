#include "pch.h"
#include "EditorRaycast.h"

#include "Engine/Scenes/Scene.h"
#include "Engine/Core/ObjectManager.h"
#include "Engine/Core/Component.h"
#include "Engine/Core/GameObject.h"
#include <Engine\Physics\Collider.h>
#include <Engine\Rendering\Renderers\Renderer.h>


namespace CurryEngine
{
	namespace EditorSupport
	{
		bool RayVsAABB(const Vector3& origin, const Vector3& direction, const Vector3& aabbMin, const Vector3& aabbMax, float& outDistance)
		{
            float tmin = 0.0f;
            float tmax = FLT_MAX;

            for (int i = 0; i < 3; ++i)
            {
                float d = (&direction.x)[i];
                float o = (&origin.x)[i];
                float lo = (&aabbMin.x)[i];
                float hi = (&aabbMax.x)[i];

                if (fabsf(d) < 1e-8f)
                {
                    // レイがこの軸に平行 → スラブ外なら即ミス
                    if (o < lo || o > hi) return false;
                }
                else
                {
                    float invD = 1.0f / d;
                    float t0 = (lo - o) * invD;
                    float t1 = (hi - o) * invD;
                    if (t0 > t1) std::swap(t0, t1);
                    tmin = std::max(tmin, t0);
                    tmax = std::min(tmax, t1);
                    if (tmin > tmax) return false;
                }
            }
            outDistance = tmin;
            return true;
		}

        bool RaycastAABBFallback(const Vector3& origin, const Vector3& direction, float maxDistance, Scene* scene, EditorRaycastResult& outResult)
        {
            bool hitAny = false;
            float closestDistance = maxDistance;
			std::shared_ptr<GameObject> closestObject = nullptr;
            for (auto& obj : scene->GetAllSceneObjects())
            {
				if (!obj || !obj->IsActive()) continue;

				// Collider コンポーネントを持つオブジェクトは無視する
                if (obj->GetComponent<Collider>()) continue;

				// オブジェクトのバウンディングボックスを取得
                for (auto& comp : obj->GetAllComponents())
                {
					if (!comp) continue;

					Math::BoundingBox aabb = comp->GetBoundingBox();
                    if (aabb.IsValid())
                    {
                        float distance;
                        Vector3 min = (aabb.min);
                        Vector3 max = (aabb.max);
                        if (RayVsAABB(origin, direction, min, max, distance))
                        {
							if (distance > 0.0f && distance < closestDistance && distance <= maxDistance)
                            {
                                hitAny = true;
                                closestDistance = distance;
                                closestObject = obj;
                            }
                        }
                    }

                    if (Renderer* renderer = dynamic_cast<Renderer*>(comp.get()))
                    {
                        Math::BoundingBox aabb = renderer->CalculateAABB();
                        if (aabb.IsValid())
                        {
                            float distance;
                            Vector3 min = (aabb.min);
                            Vector3 max = (aabb.max);
                            if (RayVsAABB(origin, direction, min, max, distance))
                            {
                                if (distance > 0.0f && distance < closestDistance && distance <= maxDistance)
                                {
                                    hitAny = true;
                                    closestDistance = distance;
                                    closestObject = obj;
                                }
                            }
                        }
					}
                }

            }
            if (hitAny)
            {
                outResult.hit = true;
				outResult.distance = closestDistance;
                outResult.hitObject = closestObject;
                return true;
            }
            else
            {
                outResult.hit = false;
                return false;
            }

        }

	}
}