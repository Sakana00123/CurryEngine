using CurryEngine;
namespace CurryEngine.Interop;

internal static unsafe class CollisionConverter
{
    public static Collision ToCollision(CollisionInfoDto* dto)
    {
        var contacts = new ContactPoint[dto->contactCount];
        
        // dto自体がすでにunsafeポインタなので直接キャストする
        ContactDto* contactDtos = (ContactDto*)dto->contacts;

        for (int i = 0; i < dto->contactCount; i++)
        {
            var src = contactDtos[i];
            contacts[i] = new ContactPoint
            {
                position = new Vector3(src.pointX, src.pointY, src.pointZ),
                normal = new Vector3(src.normalX, src.normalY, src.normalZ),
                separation = src.separation,
                thisCollider = new Collider(),
                otherCollider = new Collider(),
            };
            contacts[i].thisCollider?.Setup(src.selfId, src.selfColliderId);
            contacts[i].otherCollider?.Setup(src.otherId, src.otherColliderId);
        }

        var collision = new Collision
        {
            impulse = new Vector3(dto->impulseX, dto->impulseY, dto->impulseZ),
            contacts = contacts,
            thisCollider = new Collider(),
            otherCollider = new Collider(),
        };
        collision.thisCollider?.Setup(dto->selfId, dto->selfColliderId);
        collision.otherCollider?.Setup(dto->otherId, dto->otherColliderId);

        return collision;
    }

    public static Trigger ToTrigger(TriggerInfoDto* dto)
    {
        var trigger = new Trigger
        {
            thisCollider = new Collider(),
            otherCollider = new Collider(),
        };
        trigger.thisCollider?.Setup(dto->selfId, dto->selfColliderId);
        trigger.otherCollider?.Setup(dto->otherId, dto->otherColliderId);

        return trigger;
    }

    public static Collider? ToCollider(TriggerInfoDto* dto)
    {
        var collider = ComponentAccess.Get<Collider>(dto->otherId);
        return collider;
    }
}
