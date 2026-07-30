// This is a generated C# script.
using CurryEngine;

public class Character : Behaviour
{
    [SerializeField] float speed = 5f;
    public bool enableJump = true;
    [SerializeField] bool enableOutputLog = true;

    // Start is called before the first frame update
    public override void Start()
    {
        
    }

    // Update is called once per frame
    public override void Update()
    {
        float inputX = Input.GetAxis(GamepadSide.Left, GamepadAxis.X);
        float inputY = Input.GetAxis(GamepadSide.Left, GamepadAxis.Y);

        Vector3 cameraForward = Camera.main != null ? Camera.main.transform.forward : transform.forward;
        Vector3 cameraRight = Camera.main != null ? Camera.main.transform.right : transform.right;
        Vector3 direction = cameraForward * inputY + cameraRight * inputX;
        direction.y = 0f; // 水平方向のみに制限

        // 移動処理
        transform.position += direction * Time.deltaTime * speed;

        // 回転処理
        if (direction != Vector3.zero)
        {
            Quaternion targetRotation = Quaternion.LookRotation(direction);
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.deltaTime * 5f);
        }

        // ジャンプ処理
        if (enableJump)
        {
            if (Input.GetKeyDown(KeyCode.Space))
            {
                // ジャンプの処理をここに追加
                if (enableOutputLog)
                {
                    Debug.Log("Jump!");
                }
                
                if (gameObject.TryGetComponent<Rigidbody>(out Rigidbody rigidbody))
                {
                    rigidbody.AddForce(Vector3.up * 5f, ForceMode.Impulse);
                }
            }
        }

    }
}
