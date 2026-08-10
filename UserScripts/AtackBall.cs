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
        var otherObject = new GameObject(collision.otherColliderId);
        if (otherObject == null)
        {
            Debug.Log("Other object is null.");
            return;
        }
        if (otherObject.TryGetComponent<Character>(out Character character))
        {
            Debug.Log("Hit Character!");
            // キャラクターにダメージを与える処理をここに追加
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
