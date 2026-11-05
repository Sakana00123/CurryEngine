// This is a generated C# script.
using CurryEngine;
using System.ComponentModel;

public class AtackBall : Behaviour
{
    [SerializeField] int damage = 10;
    [SerializeField] float lifetime = 5f;
    Vector3 initialImpact = Vector3.zero;
    bool firstImpactApplied = false;
    // Start is called before the first frame update
    public override void Start()
    {
        firstImpactApplied = false;
    }

    // Update is called once per frame
    public override void Update()
    {
        if (!firstImpactApplied)
        {
            var rigidbody = GetComponent<Rigidbody>();
            if (rigidbody != null)
            {
                rigidbody.AddForce(initialImpact, ForceMode.Impulse);
                firstImpactApplied = true;
                Debug.Log("Initial impact applied: " + initialImpact);
            }
        }

        lifetime -= Time.DeltaTime;
        if (lifetime <= 0f)
        {
            Destroy(gameObject);
            //Destroy(this);
        }
    }

    public override void OnCollisionEnter(Collision collision)
    {
        var otherCollider = GetComponentById<Collider>(collision.otherColliderId);
        if (otherCollider == null)
        {
            Debug.LogWarning("Other collider not found for ID: " + collision.otherColliderId);
            return;
        }
        var otherObject = otherCollider.gameObject;
        if (otherObject == null)
        {
            Debug.Log("Other object is null.");
            return;
        }
        if (otherObject.TryGetComponent<Enemy>(out var enemy))
        {
            enemy.TakeDamage(damage);
            Debug.Log("Enemy hit! Damage: " + damage);
            Destroy(gameObject);
        }
        else
        {
            Debug.LogWarning("Collided with non-enemy object: " + otherObject.name);
        }
    }

    /// <summary>
    /// 最初の衝撃ベクトルを設定します。これにより、攻撃ボールが発射されたときの初期の衝撃方向を指定できます。
    /// </summary>
    /// <param name="impact"> 衝撃ベクトル</param>
    public void SetInitialImpact(Vector3 impact)
    {
        initialImpact = impact;
        Debug.Log("Initial impact set to: " + initialImpact);
    }
}
