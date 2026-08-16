// This is a generated C# script.
using CurryEngine;
using System.Runtime.CompilerServices;

public class Enemy : Behaviour
{
    public GameObject? playerObject;
    [SerializeField] float speed = 3f;


    // プレイヤーを追いかける距離
    [SerializeField] float chaseDistance = 10f;

    // 近づいたら静止する距離
    [SerializeField] float stopDistance = 5f;

    [SerializeField] int health = 100;
    [SerializeField] int attackDamage = 10;

    float timeSinceLastLog = 0f;

    public int AttackDamage
    {
        get => attackDamage;
        private set => attackDamage = value;
    }


    // Start is called before the first frame update
    public override void Start()
    {

    }

    // Update is called once per frame
    public override void Update()
    {
        if (timeSinceLastLog >= 2f)
        {
            float memoryUsageMB = GC.GetTotalMemory(false) / (1024f * 1024f);
            string memoryUsageString = memoryUsageMB.ToString("F2");
            Debug.Log($"GC.GetTotalMemory(): {memoryUsageString} MB");
            timeSinceLastLog = 0f;
        }
        else
        {
            timeSinceLastLog += Time.DeltaTime;
        }


        // 奈落に落ちた場合は死亡処理を行う
        if (transform.position.y < -10f)
        {
            Die();
        }

        // プレイヤーオブジェクトがまだ見つかっていない場合は、Findで探す
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
                transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.DeltaTime * 5f);
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

            Vector3 movement = directionToPlayer.normalized * speed;

            // プレイヤーに向かって移動する
            //transform.position += movement * Time.DeltaTime;
            if (TryGetComponent<Rigidbody>(out Rigidbody rigidbody))
            {
                rigidbody.AddForce(movement, ForceMode.Force);
            }
        }
    }


    public void TakeDamage(int damage)
    {
        health -= damage;
        Debug.Log($"Enemy took {damage} damage. Remaining health: {health}");
        if (health <= 0)
        {
            Die();
        }
    }

    private void Die()
    {
        Debug.Log("Enemy died.");
        Destroy(gameObject);
    }

}
