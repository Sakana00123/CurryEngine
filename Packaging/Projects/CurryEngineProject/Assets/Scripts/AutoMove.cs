// This is a generated C# script.
using CurryEngine;
using System.ComponentModel;

public class AutoMove : Behaviour
{
    [SerializeField] private float speed = 100.0f;
    [SerializeField] private float range = 5.0f;
    [SerializeField] private Vector3 direction = Vector3.forward;
    [SerializeField] private Vector3 center = Vector3.zero;
    [SerializeField] private float time = 0.0f;

    // Start is called before the first frame update
    public override void Start()
    {
        time = 0.0f;
    }

    // Update is called once per frame
    public override void Update()
    {
        // Circle movement
        time += Time.DeltaTime;

        // 周囲を回る動き
        //Vector3 newPosition = center + Quaternion.Euler(0, time * speed, 0) * direction.normalized * range;

        // 進み続ける動き
        Vector3 newPosition = transform.position + direction.normalized * speed * Time.DeltaTime;

        // 座標更新
        transform.position = newPosition;

    }
}
