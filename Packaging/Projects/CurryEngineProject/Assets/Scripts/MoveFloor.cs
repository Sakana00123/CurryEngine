// This is a generated C# script.
using CurryEngine;

public class MoveFloor : Behaviour
{
    [SerializeField] Vector3 startPosition;
    [SerializeField] Vector3 endPosition;
    [SerializeField] float speed;
    float time;

    // Start is called before the first frame update
    public override void Start()
    {
    }

    // Update is called once per frame
    public override void Update()
    {
        time += Time.DeltaTime;
        // サイン波を使って、startPositionとendPositionの間を往復する動きを作る
        float t = (CurryEngine.Math.Mathf.Sin(time * speed) + 1.0f) / 2.0f;

        // 等速で往復する動きを作る
        //float t = CurryEngine.Math.Mathf.PingPong(time * speed, 1.0f);

        transform.position = Vector3.Lerp(startPosition, endPosition, t);
    }
}
