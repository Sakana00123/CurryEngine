// This is a generated C# script.
using CurryEngine;

public class PlayerController : Behaviour
{
    GltfModelRenderer? modelRenderer;
    public float speed = 0.0f;
    // Start is called before the first frame update
    public override void Start()
    {
        modelRenderer = GetComponent<GltfModelRenderer>();
    }

    // Update is called once per frame
    public override void Update()
    {
        //if (modelRenderer != null)
        //{
        //    Vector2 input = Input.GetAxis(GamepadStick.LeftStick);
        //    //modelRenderer.SetAnimationParameter("InputX", input.x);
        //    //modelRenderer.SetAnimationParameter("InputY", input.y);
        //    if (Input.GetKey(KeyCode.F))
        //    {
        //        modelRenderer.PlayAnimation(1);
        //    }
        //    if (Input.GetKey(KeyCode.G))
        //    {
        //        modelRenderer.PlayAnimation(2);
        //    }
        //    if (Input.GetKey(KeyCode.H))
        //    {
        //        modelRenderer.PlayAnimation(3);
        //    }
        //    if (Input.GetKey(KeyCode.J))
        //    {
        //        modelRenderer.PlayAnimation(4);
        //    }
        //}
        Vector2 input = Input.GetAxis(GamepadStick.LeftStick);
        Vector3 movement = new Vector3(input.x, 0, input.y);
        speed = movement.magnitude;
        transform.Translate(movement.normalized * speed * Time.DeltaTime);

    }
}
