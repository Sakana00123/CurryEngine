// This is a generated C# script.
using CurryEngine;

public class AttackController : Behaviour
{
    public Collider? attackCollider;
    public int attackCount = 0;
    // Start is called before the first frame update
    public override void Start()
    {
        
    }

    // Update is called once per frame
    public override void Update()
    {
        
    }

    public override void OnTriggerStay(Trigger trigger)
    {
        var other = GetComponentById<SphereCollider>(trigger.otherColliderId);
        var otherGameObject = other?.gameObject;
        if (otherGameObject != null)
        {
            if (otherGameObject.name.Contains("Enemy"))
            {
                Debug.Log("PlayerController: Colliding with enemy: " + otherGameObject.name);

                // 攻撃判定中に敵と接触している場合の処理
                if (attackCollider != null && attackCollider.Enabled && attackCount > 0)
                {
                    // 攻撃処理を行う
                    attackCount--;
                    Debug.Log("PlayerController: Attacking enemy! Attack count: " + attackCount);
                    // ここで敵にダメージを与える処理を追加することができます
                    otherGameObject.GetComponent<Enemy>()?.TakeDamage(10);
                }
            }
        }
    }
}
