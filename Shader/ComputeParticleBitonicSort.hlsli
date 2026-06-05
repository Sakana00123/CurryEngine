RWStructuredBuffer<ParticleHeader> particleHeaderBuffer : register(u3);

bool comparer(in ParticleHeader x0, in ParticleHeader x1)
{
    //  ¶‘¶‚Í+1AŽ€–S‚Í0
    //  “¯‚¶ƒtƒ‰ƒO‚Å‚ ‚ê‚Î[“x”äŠr
    return (x0.alive > x1.alive || (x0.alive == x1.alive && x0.depth > x1.depth));
}