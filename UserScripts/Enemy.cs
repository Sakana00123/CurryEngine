// This is a generated C# script.
using CurryEngine;

public class Enemy : Behaviour
{
    public GameObject? playerObject;
    [SerializeField] float speed = 3f;


    // プレイヤーを追いかける距離
    [SerializeField] float chaseDistance = 10f;

    // 近づいたら静止する距離
    [SerializeField] float stopDistance = 5f;

    float timeSinceLastLog = 0f;

    // Start is called before the first frame update
    public override void Start()
    {
        
    }

    // Update is called once per frame
    public override void Update()
    {
        if (timeSinceLastLog >= 2f)
        {
            Debug.Log($"GC.GetTotalAllocatedBytes(): {GC.GetTotalAllocatedBytes()} bytes");
            timeSinceLastLog = 0f;
        }
        else
        {
            timeSinceLastLog += Time.deltaTime;
        }

        if (playerObject == null)
        {
            if (Find("character") is GameObject player)
            {
                playerObject = player;
                Debug.Log($"Player object found: {player.name}");
            }
            else
            {
                Debug.LogWarning("Player object not found.");
                return;
            }
        }
        else
        {
            // プレイヤーの方向を向く
            Vector3 directionToPlayer = playerObject.transform.position - transform.position;
            directionToPlayer.y = 0f; // 水平方向のみに制限

            if (directionToPlayer != Vector3.zero)
            {
                Quaternion targetRotation = Quaternion.LookRotation(directionToPlayer);
                transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.deltaTime * 5f);
            }

            // プレイヤーとの距離を計算
            float distanceToPlayer = directionToPlayer.magnitude;

            if (distanceToPlayer > chaseDistance) {
                // プレイヤーが追いかける距離より遠い場合は何もしない
                return;
            }
            if (distanceToPlayer < stopDistance) {
                // プレイヤーが静止する距離より近い場合は何もしない
                return;
            }

            Vector3 movement = directionToPlayer.normalized * speed * Time.deltaTime;

            // プレイヤーに向かって移動する
            transform.position += movement;
        }
    }
}
