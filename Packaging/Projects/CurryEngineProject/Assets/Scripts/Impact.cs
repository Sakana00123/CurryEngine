// This is a generated C# script.
using CurryEngine;

public class Impact : Behaviour
{
    [SerializeField] private Vector3 force = new Vector3(0, 5.0f, 0);
    Rigidbody? rb;

    // Start is called before the first frame update
    public override void Start()
    {
        rb = GetComponent<Rigidbody>();
    }

    // Update is called once per frame
    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            if (rb != null)
            {
                rb.AddForce(force, ForceMode.Impulse);
            }
        }
    }
}
