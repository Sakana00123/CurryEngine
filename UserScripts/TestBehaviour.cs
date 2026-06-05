
using CurryEngine;
using CurryEngine.Math;

public class TestBehaviour : Behaviour
{
    [SerializeField] private Vector3 initialPos = new Vector3(3, 3, 3);
    [SerializeField] private float speed = 2.0f;
    [SerializeField] private float volume = 0.3f;
    [SerializeField] private AudioSource? audioSource;

    [SerializeField] Rigidbody? rigidbody = null;

    public override void OnEnable()
    {
        Debug.Log("OnEnable!");
    }

    public override void Start()
    {
        Debug.Log("TestBehaviour Start!");
        //File.AppendAllText("debug.txt", Behaviour.DebugAccessorInfo() + "\n");
        transform.position = initialPos;
        transform.rotation = Quaternion.identity;

        // Rigidbodyコンポーネントを取得して保存
        //rigidbody = GetComponent<Rigidbody>();
    }
    public override void Update()
    {
        //Debug.Log("TestBehaviour Update!");
        Vector3 translation = new Vector3(1f, 0f, 1f);
        //transform.Translate(translation * speed * Time.deltaTime);

        float inputX = Input.GetAxis(GamepadSide.Left, GamepadAxis.X);
        float inputY = Input.GetAxis(GamepadSide.Left, GamepadAxis.Y);

        translation.x = inputX * speed * Time.deltaTime;
        translation.z = inputY * speed * Time.deltaTime;

        if (Mathf.Abs(inputX) > Mathf.Epsilon || Mathf.Abs(inputY) > Mathf.Epsilon)
        {
            transform.Translate(translation);
        }

        // Rigidbodyが存在する場合はジャンプが押されたら上方向に力を加える
        if (rigidbody != null && Input.GetKeyDown(KeyCode.Space))
        {
            //rigidbody.AddForce(new Vector3(0f, 5f, 0f), ForceMode.Impulse);
            Debug.Log("Jump!");
        }

        audioSource?.SetVolume(volume);

    }

    public void TestFunc()
    {
        Debug.Log("Called TestFunc!");
    }


    public override void OnCollisionStay(Collision collision)
    {
        //Debug.Log("CollisionStay!");
        //Debug.Log($"Impulse: {collision.impulse}, ContactCount: {collision.contacts.Length}");
        for (int i = 0; i < collision.contacts.Length; i++)
        {
            var contact = collision.contacts[i];
            //Debug.Log($"Contact {i}: Collider:{contact.otherCollider} Position={contact.position}, Normal={contact.normal}, Separation={contact.separation}");
        }
    }

    public override void OnTriggerStay(Trigger trigger)
    {
        //Debug.Log("TriggerStay!");
        //Debug.Log($"otherCollider: {trigger.otherCollider}");
    }


    public override void OnDestroy()
    {
        Debug.Log("TestBehaviour OnDestroy!");
    }
}
