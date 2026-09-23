// This is a generated C# script.
using CurryEngine;

public class PlayerController : Behaviour
{
    Animator? animator;
    public float speed = 0.0f;
    public float acceleration = 5.0f;
    public float jumpForce = 5.0f;
    public GameObject? attackColliderObject;
    Vector2 prevInput = Vector2.zero;
    int jumpCount = 0;
    int attackCount = 0;
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

        // 移動処理
        if (animator != null)
        {
            if (animator.GetCurrentStateIndex() != animator.GetStateIndexFromName("Attack"))
            {
                bool isRunning = Input.GetKey(KeyCode.LeftShift);
                speed = direction.magnitude * acceleration;
                if (isRunning)
                {
                    speed *= 2.0f; // Double the speed when running
                }
                transform.Translate(direction.normalized * speed * Time.DeltaTime);
            }
            else
            {
                speed = 0.0f;
            }
        }

        // 回転処理
        if (direction != Vector3.zero)
        {
            Vector3 forward = cameraForward;
            forward.y = 0f; // 水平方向のみに制限
            Quaternion targetRotation = Quaternion.LookRotation(forward);
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.DeltaTime * 5f);
        }

        // 接地判定
        if (TryGetComponent<Rigidbody>(out Rigidbody rigidbody))
        {
            Vector3 velocity = rigidbody.GetVelocity();
            // 下降中のみ接地判定を行う
            if (velocity.y <= 0.0f)
            {
                // 接地判定のためのレイキャスト
                Vector3 origin = transform.position + Vector3.up * 0.1f; // 少し上からレイを飛ばす
                Vector3 directionDown = Vector3.down;
                float maxDistance = 0.15f; // 接地判定の距離
                LayerMask layerMask = LayerMask.NameToLayer("Default"); // 接地判定を行うレイヤーを指定
                if (Physics.Raycast(origin, directionDown, out RaycastHit hitInfo, maxDistance, layerMask))
                {
                    var hitCollider = GetComponentById<Collider>(hitInfo.colliderId);
                    if (hitCollider != null)
                    {
                        OnGround();
                    }
                    else
                    {
                        Debug.LogWarning("PlayerController: Raycast hit unknown collider with ID " + hitInfo.colliderId);
                    }
                }
            }
        }

        // アニメーションの更新
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
                    //animator.SetTrigger("JumpTrigger");
                    animator.SetBool("Jumping", true);
                    animator.CrossFadeInFixedTime("JumpStart", 0.1f);
                    jumpCount++;
                }
            }
        }
        if (input != Vector2.zero)
        { 
            prevInput = input;
        }
    }

    void OnGround()
    {
        jumpCount = 0;
        if (animator != null)
        {
            animator.SetBool("Jumping", false);
        }
    }

    public void Test()
    {
        Debug.Log("PlayerController: Test method called");
    }

    private void SetColliderEnabled(bool enabled)
    {
        if (attackColliderObject != null)
        {
            if (attackColliderObject.TryGetComponent<Collider>(out Collider collider))
            {
                collider.Enabled = enabled;
                Debug.Log($"PlayerController: Attack collider {collider.gameObject.name} enabled set to {enabled}");
            }
            if (enabled)
            {
                ResetAttackCount();
            }
            else
            {
            }
        }
    }

    public void EnableAttackCollider()
    {
        SetColliderEnabled(true);
    }

    public void DisableAttackCollider()
    {
        SetColliderEnabled(false);
    }

    public void ResetAttackCount()
    {
        if (attackColliderObject != null && attackColliderObject.TryGetComponent<AttackController>(out AttackController attackController))
        {
            attackController.attackCount = 1;
            Debug.Log($"AttackController: Reset attack count to {attackController.attackCount}");
        }
    }

    //private void OnGui()
    //{
    //    if (animator != null)
    //    {
    //        GUILayout.Label("Speed: " + speed.ToString("F2"));
    //        GUILayout.Label("Jump Count: " + jumpCount);
    //        GUILayout.Label("Is Jumping: " + animator.GetBool("Jumping"));
    //    }
    //}

    //public override void OnCollisionEnter(Collision collision)
    //{
    //    Collider? other = GetComponentById<Collider>(collision.otherColliderId);
    //    var otherGameObject = other?.gameObject;
    //    if (otherGameObject != null)
    //    {
    //        if (otherGameObject.name == "Ground")
    //        {
    //            Debug.Log("PlayerController: OnCollisionEnter with Ground");
    //            jumpCount = 0;
    //            if (animator != null)
    //            {
    //                animator.SetBool("Jumping", false);
    //            }
    //        }
    //    }
    //}
}
