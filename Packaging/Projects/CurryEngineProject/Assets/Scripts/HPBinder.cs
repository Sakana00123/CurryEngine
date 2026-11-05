// This is a generated C# script.
using CurryEngine;

public class HPBinder : Behaviour
{
    [SerializeField] Character? character;
    [SerializeField] RectTransform? hpBarRectTransform;
    int maxHealth = 100;
    Vector2 originalSize = new Vector2(200f, 20f); // HPバーの元のサイズ

    // Start is called before the first frame update
    public override void Start()
    {
        if (character != null)
        {
            maxHealth = character.health;
            character.OnHealthChanged += UpdateHPBar;
        }
        else
        {
            Debug.LogWarning("[HPBinder] Character reference is not set.");
        }
        if (hpBarRectTransform != null)
        {
            originalSize = hpBarRectTransform.size;
        }
        else
        {
            Debug.LogWarning("[HPBinder] HP bar RectTransform reference is not set.");
        }
    }

    // Update is called once per frame
    public override void Update()
    {
        
    }


    private void UpdateHPBar(int currentHealth)
    {
        Debug.Log($"[HPBinder] Updating HP bar: {currentHealth}/{maxHealth}");
        if (hpBarRectTransform != null)
        {
            float healthPercentage = (float)currentHealth / maxHealth;
            hpBarRectTransform.size = new Vector2(originalSize.x * healthPercentage, originalSize.y);
        }
        else
        {
            Debug.LogWarning("[HPBinder] HP bar RectTransform reference is not set.");
        }
    }

}
