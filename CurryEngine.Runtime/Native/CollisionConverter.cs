namespace CurryEngine.Runtime.Native;

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
                thisColliderId = src.selfColliderId,
                otherColliderId = src.otherColliderId,
            };
        }

        var collision = new Collision
        {
            impulse = new Vector3(dto->impulseX, dto->impulseY, dto->impulseZ),
            contacts = contacts,
            thisColliderId = dto->selfColliderId,
            otherColliderId = dto->otherColliderId,
        };

        return collision;
    }

    public static Trigger ToTrigger(TriggerInfoDto* dto)
    {
        var trigger = new Trigger
        {
            thisColliderId = dto->selfColliderId,
            otherColliderId = dto->otherColliderId
        };

        return trigger;
    }
}
