// This is a generated C# script.
using CurryEngine;

public class PlayerController : Behaviour
{
    Animator? animator;
    public float speed = 0.0f;
    // Start is called before the first frame update
    public override void Start()
    {
        animator = GetComponent<Animator>();
    }

    // Update is called once per frame
    public override void Update()
    {
        Vector2 input = Input.GetAxis(GamepadStick.LeftStick);
        Vector3 movement = new Vector3(input.x, 0, input.y);
        speed = movement.magnitude;
        transform.Translate(movement.normalized * speed * Time.DeltaTime);
        if (animator != null)
        {
            animator.SetFloat("Speed", speed);

            if (Input.GetKeyDown(KeyCode.F))
            {
                animator.SetTrigger("AttackTrigger");
            }
        }
    }
}
