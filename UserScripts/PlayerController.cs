// This is a generated C# script.
using CurryEngine;

public class PlayerController : Behaviour
{
    Animator? animator;
    public float speed = 0.0f;
    public float acceleration = 5.0f;
    public float jumpForce = 5.0f;
    Vector2 prevInput = Vector2.zero;
    int jumpCount = 0;
    // Start is called before the first frame update
    public override void Start()
    {
        animator = GetComponent<Animator>();
    }

    // Update is called once per frame
    public override void Update()
    {
        Vector2 input = Input.GetAxis(GamepadStick.LeftStick);

        Vector3 cameraForward = Camera.main != null ? Camera.main.transform.forward : transform.forward;
        Vector3 cameraRight = Camera.main != null ? Camera.main.transform.right : transform.right;
        Vector3 direction = cameraForward * input.y + cameraRight * input.x;
        direction.y = 0f; // 水平方向のみに制限
        bool isRunning = Input.GetKey(KeyCode.LeftShift);
        speed = direction.magnitude * acceleration;
        if (isRunning)
        {
            speed *= 2.0f; // Double the speed when running
        }
        transform.Translate(direction.normalized * speed * Time.DeltaTime);

        // 回転処理
        if (direction != Vector3.zero)
        {
            Vector3 forward = cameraForward;
            forward.y = 0f; // 水平方向のみに制限
            Quaternion targetRotation = Quaternion.LookRotation(forward);
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.DeltaTime * 5f);
        }

        if (animator != null)
        {
            animator.SetFloat("Speed", speed);
            animator.SetFloat("InputX", input.x);
            animator.SetFloat("InputY", input.y);
            animator.SetFloat("PrevInputX", prevInput.x);
            animator.SetFloat("PrevInputY", prevInput.y);

            if (Input.GetKeyDown(KeyCode.F))
            {
                animator.SetTrigger("AttackTrigger");
            }
            if (Input.GetKeyDown(KeyCode.Space))
            {
                if (jumpCount < 2)
                {
                    if (TryGetComponent<Rigidbody>(out Rigidbody rb))
                    {
                        Vector3 velocity = rb.GetVelocity();
                        velocity.y = jumpForce;
                        rb.SetVelocity(velocity);
                    }
                    animator.SetBool("Jumping", true);
                    jumpCount++;
                }
            }
        }
        if (input != Vector2.zero)
        { 
            prevInput = input;
        }
    }

    public override void OnCollisionEnter(Collision collision)
    {
        Collider? other = GetComponentById<Collider>(collision.otherColliderId);
        var otherGameObject = other?.gameObject;
        if (otherGameObject != null)
        {
            if (otherGameObject.name == "Ground")
            {
                jumpCount = 0;
                if (animator != null)
                {
                    animator.SetBool("Jumping", false);
                }
            }
        }
    }
}
