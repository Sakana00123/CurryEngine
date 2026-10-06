// This is a generated C# script.
using CurryEngine;

public class Rotater : Behaviour
{
    [SerializeField] private float rotationSpeed = 90.0f;
    [SerializeField] private Vector3 rotationAxis = Vector3.forward;

    // Start is called before the first frame update
    public override void Start()
    {
        
    }

    // Update is called once per frame
    public override void Update()
    {
        transform.Rotate(rotationAxis, rotationSpeed * Time.DeltaTime);
    }
}
