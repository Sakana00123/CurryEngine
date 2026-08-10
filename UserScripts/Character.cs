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
    [SerializeField] GameObject? attackTarget;

    // Start is called before the first frame update
    public override void Start()
    {
        
    }

    // Update is called once per frame
    public override void Update()
    {
        Vector2 input = Input.GetAxis(GamepadStick.LeftStick);

        Vector3 cameraForward = Camera.main != null ? Camera.main.transform.forward : transform.forward;
        Vector3 cameraRight = Camera.main != null ? Camera.main.transform.right : transform.right;
        Vector3 direction = cameraForward * input.y + cameraRight * input.x;
        direction.y = 0f; // 水平方向のみに制限

        // 移動処理
        transform.position += direction * Time.DeltaTime * speed;

        // 回転処理
        if (direction != Vector3.zero)
        {
            Quaternion targetRotation = Quaternion.LookRotation(direction);
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.DeltaTime * 5f);
        }

        // ジャンプ処理
        if (enableJump)
        {
            if (Input.GetKeyDown(KeyCode.Space))
            {
                // ジャンプの処理をここに追加
                if (gameObject.TryGetComponent<Rigidbody>(out Rigidbody rigidbody))
                {
                    Debug.Log("Jump!");
                    rigidbody.SetVelocity(Vector3.up * jumpForce);
                }
            }
        }

        // 攻撃処理
        if (enableAttack)
        {
            if (Input.GetKeyDown(KeyCode.MouseLeft))
            {
                // 攻撃の処理をここに追加
                if (attackPoint != null)
                {
                    //GameObject? attackPrefab = Resources.Load<GameObject>(attackPrefabPath);
                    if (Path.Exists(attackPrefabPath))
                    {
                        GameObject attackInstance = Instantiate(attackPrefabPath, attackPoint.transform.position, attackPoint.transform.rotation);
                        Debug.Log($"Instantiated: {attackInstance.name}/{attackInstance.ToString()}");

                        Vector3 attackDir = (attackTarget != null) ? (attackTarget.transform.position - attackPoint.transform.position).normalized : attackPoint.transform.forward;
                        attackDir.y = 0f; // 水平方向のみに制限
                        attackDir = attackDir.normalized; // 正規化して方向ベクトルにする
                        if (attackInstance.TryGetComponent<AtackBall>(out AtackBall attackBall))
                        {
                            attackBall.SetInitialImpact(attackDir * attackForce);
                        }
                        else
                        {
                            Debug.LogError("Attack instance does not have AtackBall component.");
                        }


                        //if (attackInstance.TryGetComponent<Rigidbody>(out Rigidbody attackRigidbody))
                        //{
                        //    Debug.Log($"Attack Direction: {attackDir}");
                        //    attackRigidbody.AddForce(attackDir * attackForce, ForceMode.Impulse);
                        //}
                    }
                    else
                    {
                        Debug.LogError($"Failed to load attack prefab from path: {attackPrefabPath}");
                    }
                }
                else
                {
                    Debug.LogError("Attack point is not assigned.");
                }
            }
        }

    }
}
