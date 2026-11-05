// This is a generated C# script.
using CurryEngine;
using CurryEngine.Math;

public class NewScript : Behaviour
{
    [SerializeField] private float speed = 5f;
    public string prefabPath = "./Assets/Prefabs/Enemy.json";
    public GameObject prefab;
    public TestBehaviour behaviour;

    // Start is called before the first frame update
    public override void Start()
    {
        var testBehaviour = GetComponent<TestBehaviour>();
        if (testBehaviour != null)
        {
            testBehaviour.TestFunc();
        }
        else
        {
            Debug.LogWarning("Not Found TestBehaviour");
        }
    }

    // Update is called once per frame
    public override void Update()
    {
        //transform.Translate(Vector3.forward * speed * Time.deltaTime);

        if (Input.GetKeyDown(KeyCode.Space))
        {
            //Instantiate(prefabPath);
            if (prefab)
            {
                Debug.Log(prefab.ToString());
                Instantiate(prefab);
            }
        }
    }
}
