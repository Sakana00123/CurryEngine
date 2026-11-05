// This is a generated C# script.
using CurryEngine;

public class Character : Behaviour
{
    [SerializeField] float speed = 5f;
    public bool enableJump = true;
    public float jumpForce = 5f;
    public bool enableAttack = true;
    [SerializeField] string attackPrefabPath = "TestAssets/Prefabs/PlayerBall.prefab";
    [SerializeField] GameObject? attackPoint;
    [SerializeField] float attackForce = 30f;
    public int health = 100;

    // 攻撃対象のTransformを指定するためのフィールド
    [SerializeField] Transform? attackTarget;

    // 効果音を再生するためのAudioSourceを指定するためのフィールド
    [SerializeField] AudioSource? attackAudioSource;

    private Action<int>? _onHealthChanged;
    public Action<int>? OnHealthChanged
    { 
        get => _onHealthChanged;
        set => _onHealthChanged = value;
    }

    // Start is called before the first frame update
    public override void Start()
    {

    }

    // Update is called once per frame
    public override void Update()
    {
        //Vector2 input = Input.GetAxis(GamepadStick.LeftStick);

        //Vector3 cameraForward = Camera.main != null ? Camera.main.transform.forward : transform.forward;
        //Vector3 cameraRight = Camera.main != null ? Camera.main.transform.right : transform.right;
        //Vector3 direction = cameraForward * input.y + cameraRight * input.x;
        //direction.y = 0f; // 水平方向のみに制限

        //// 移動処理
        //transform.position += direction * Time.DeltaTime * speed;

        //// 回転処理
        //if (direction != Vector3.zero)
        //{
        //    Quaternion targetRotation = Quaternion.LookRotation(direction);
        //    transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.DeltaTime * 5f);
        //}

        //// ジャンプ処理
        //if (enableJump)
        //{
        //    if (Input.GetKeyDown(KeyCode.Space))
        //    {
        //        // ジャンプの処理をここに追加
        //        if (gameObject.TryGetComponent<Rigidbody>(out Rigidbody rigidbody))
        //        {
        //            Debug.Log("Jump!");
        //            rigidbody.SetVelocity(Vector3.up * jumpForce);
        //        }
        //    }
        //}

        //// 攻撃処理
        //if (enableAttack)
        //{
        //    if (Input.GetKeyDown(KeyCode.MouseLeft))
        //    {
        //        // 攻撃の処理をここに追加
        //        if (attackPoint != null)
        //        {
        //            //GameObject? attackPrefab = Resources.Load<GameObject>(attackPrefabPath);
        //            if (Path.Exists(attackPrefabPath))
        //            {
        //                GameObject attackInstance = Instantiate(attackPrefabPath, attackPoint.transform.position, attackPoint.transform.rotation);
        //                if (!attackTarget)
        //                {
        //                    attackTarget = FindAttackTarget();
        //                }
        //                Vector3 attackDir = (attackTarget != null) ? (attackTarget.position - attackPoint.transform.position).normalized : attackPoint.transform.forward;
        //                attackDir.y = 0f; // 水平方向のみに制限
        //                attackDir = attackDir.normalized; // 正規化して方向ベクトルにする
        //                if (attackInstance.TryGetComponent<AtackBall>(out AtackBall attackBall))
        //                {
        //                    attackBall.SetInitialImpact(attackDir * attackForce);

        //                    // 攻撃音を再生する
        //                    if (attackAudioSource != null)
        //                    {
        //                        attackAudioSource.PlayOneShot();
        //                    }
        //                }
        //                else
        //                {
        //                    Debug.LogError("Attack instance does not have AtackBall component.");
        //                }


        //                //if (attackInstance.TryGetComponent<Rigidbody>(out Rigidbody attackRigidbody))
        //                //{
        //                //    Debug.Log($"Attack Direction: {attackDir}");
        //                //    attackRigidbody.AddForce(attackDir * attackForce, ForceMode.Impulse);
        //                //}
        //            }
        //            else
        //            {
        //                Debug.LogError($"Failed to load attack prefab from path: {attackPrefabPath}");
        //            }
        //        }
        //        else
        //        {
        //            Debug.LogError("Attack point is not assigned.");
        //        }
        //    }
        //}

    }


    Transform? FindAttackTarget()
    {
        // ここで攻撃対象を見つけるロジックを実装します。
        if (attackTarget == null)
        {
            Transform? closestEnemy = null;
            float closestDistance = float.MaxValue;
            GameObject[] enemies = FindAllByType<Enemy>();
            Debug.Log($"[FindAttackTarget] Found {enemies.Length} enemies.");
            foreach (var obj in enemies)
            {
                //if (obj.CompareTag("Enemy")) // 例えば、攻撃対象のタグが "Enemy" の場合
                if (obj.transform)
                {
                    float distance = Vector3.Distance(transform.position, obj.transform.position);
                    if (distance < closestDistance)
                    {
                        closestDistance = distance;
                        closestEnemy = obj.transform;
                    }
                }
            }
            if (closestEnemy != null) {
                Debug.Log($"[FindAttackTarget] Found closest enemy at distance: {closestDistance}");
            } else {
                Debug.Log("[FindAttackTarget] No enemies found.");
            }
            return closestEnemy;
        }
        return null;
    }
    
    public void TakeDamage(int damage)
    {
        health -= damage;
        OnHealthChanged?.Invoke(health);
        Debug.Log($"Character took {damage} damage. Remaining health: {health}");
        if (health <= 0)
        {
            Debug.Log("Character has died.");
        }
    }

    public override void OnCollisionEnter(Collision collision)
    {
        Collider? other = GetComponentById<Collider>(collision.otherColliderId);
        var otherGameObject = other?.gameObject;
        if (otherGameObject != null)
        {
            if (otherGameObject.TryGetComponent<Enemy>(out Enemy enemy))
            {
                Debug.Log($"Collided with Enemy: {otherGameObject.name}");
                TakeDamage(enemy.AttackDamage);
            }
        }
    }

}
