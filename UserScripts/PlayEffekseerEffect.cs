// This is a generated C# script.
using CurryEngine;

public class PlayEffekseerEffect : Behaviour
{
    [SerializeField] bool playOnStart = true; // Whether to play the effect on start
    EffekseerEffectComponent? effekseerEffectComponent; // Reference to the EffekseerEffectComponent


    // Start is called before the first frame update
    public override void Start()
    {
        if (TryGetComponent<EffekseerEffectComponent>(out effekseerEffectComponent))
        {
            if (playOnStart)
            {
                effekseerEffectComponent.PlayEffect();
            }
        }
    }

    // Update is called once per frame
    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            effekseerEffectComponent?.PlayEffect();
        }
    }
}
