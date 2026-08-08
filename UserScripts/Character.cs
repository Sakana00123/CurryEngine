// This is a generated C# script.
using CurryEngine;

public class Character : Behaviour
{
    [SerializeField] float speed = 5f;
    public bool enableJump = true;

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
                    rigidbody.SetVelocity(Vector3.up * 5f);
                }
            }
        }

    }
}
