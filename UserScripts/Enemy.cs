// This is a generated C# script.
using CurryEngine;
using CurryEngine.Math;
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
    [SerializeField] float attackCooldown = 3f;
    [SerializeField] float knockbackForce = 5f;
    Animator? animator;

    float attackTimer = 0f;

    bool isDead = false;

    public int AttackDamage
    {
        get => attackDamage;
        private set => attackDamage = value;
    }


    // Start is called before the first frame update
    public override void Start()
    {
        animator = GetComponent<Animator>();
    }

    // Update is called once per frame
    public override void Update()
    {
        if (isDead)
        {
            return; // 死亡している場合は何もしない
        }
        // 奈落に落ちた場合は死亡処理を行う
        if (transform.position.y < -10f)
        {
            Die();
        }

        if (Input.GetKeyDown(KeyCode.K))
        {
            TakeDamage(20);
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
                if (animator != null)
                {
                    animator.SetFloat("Speed", 0.0f);
                }
                return;
            }
            if (distanceToPlayer < stopDistance) {
                // プレイヤーが静止する距離より近い場合は何もしない
                if (animator != null)
                {
                    animator.SetFloat("Speed", 0.0f);
                }
                return;
            }

            Vector3 movement = directionToPlayer.normalized * speed;

            // プレイヤーに向かって移動する
            transform.position += movement * Time.DeltaTime;
            if (animator != null)
            {
                animator.SetFloat("Speed", movement.magnitude);
            }

            //if (TryGetComponent<Rigidbody>(out Rigidbody rigidbody))
            //{
            //    rigidbody.AddForce(movement, ForceMode.Force);
            //}

            // 攻撃処理
            if (distanceToPlayer <= stopDistance && attackTimer >= attackCooldown)
            {
                // プレイヤーに攻撃する処理をここに追加
                Debug.Log($"Enemy attacks the player for {attackDamage} damage!");
                if (playerObject.TryGetComponent<Character>(out Character player))
                {
                    player.TakeDamage(attackDamage);
                }
                attackTimer = 0f;
                if (animator != null)
                {
                    animator.SetTrigger("AttackTrigger");
                }
            }
            else
            {
                attackTimer += Time.DeltaTime;
            }
        }
    }

    // ダメージを受ける処理
    public void TakeDamage(int damage)
    {
        if (isDead)
        {
            return; // すでに死亡している場合はダメージを受けない
        }
        health -= damage;
        if (health > 0)
        {
            // ダメージを受けたときのアニメーションを再生する
            if (animator != null)
            {
                animator.SetTrigger("HitTrigger");
            }
        }
        if (TryGetComponent<Rigidbody>(out Rigidbody rigidbody))
        {
            // ダメージを受けたときに少し後ろに吹き飛ばす
            Vector3 knockbackDirection = -transform.forward;
            rigidbody.AddForce(knockbackDirection * knockbackForce, ForceMode.Impulse);

            Debug.Log($"Enemy knocked back with force: {knockbackDirection * knockbackForce}");
        }

        Debug.Log($"Enemy took {damage} damage. Remaining health: {health}");
        if (health <= 0)
        {
            Die();
        }
    }

    // 敵が死亡する処理
    private void Die()
    {
        Debug.Log("Enemy died.");
        isDead = true;

        if (animator != null)
        {
            animator.SetTrigger("DeathTrigger");
        }

        Destroy(gameObject, 4f);
    }

}
