// This is a generated C# script.
using CurryEngine;
using System.ComponentModel;

public class Spawner : Behaviour
{
    [SerializeField] string prefabPath = "TestAssets/Prefabs/Enemy.prefab";
    [SerializeField] float spawnInterval = 5f;
    float spawnTimer = 0f;

    // Start is called before the first frame update
    public override void Start()
    {
        spawnTimer = spawnInterval; // Initialize the spawn timer
    }

    // Update is called once per frame
    public override void Update()
    {
        spawnTimer -= Time.DeltaTime;
        if (spawnTimer <= 0f)
        {
            GameObject? spawnedObject = Instantiate(prefabPath, transform.position, Quaternion.identity);
            if (spawnedObject != null)
            {
                Debug.Log($"Spawned object: {spawnedObject.name}");
            }
            else
            {
                Debug.LogWarning($"Failed to spawn object from prefab path: {prefabPath}");
            }
            spawnTimer = spawnInterval; // Reset the spawn interval
        }
    }
}
