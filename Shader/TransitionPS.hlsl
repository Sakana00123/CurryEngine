#include "fullscreen_quad.hlsli"
#include "Sampler.hlsli"
#include "Constants.hlsli"



cbuffer TransitionBuffer : register(b6)
{
    float transitionProgress; // 0.0 (無変化) ～ 1.0 (完全遷移)
    int isFadeIn; // false: 画面->黒 (OUT) / true: 黒->画面 (IN)
};

Texture2D textureMap : register(t0); // 入力シーン画像

#define HEXTILE_SIZE 0.125
#define RANDOMNESS   0.75

// --- ヘルパー関数群 ---
float hash(float2 co)
{
    return frac(sin(dot(co, float2(12.9898, 58.233))) * 13758.5453);
}

float tanh_approx(float x)
{
    float x2 = x * x;
    return clamp(x * (27.0 + x2) / (27.0 + 9.0 * x2), -1.0, 1.0);
}

float2 mod2(float2 x, float2 y)
{
    return x - y * floor(x / y);
}

float hex(float2 p, float r)
{
    p.xy = p.yx;
    const float3 k = float3(-sqrt(3.0 / 4.0), 1.0 / 2.0, 1.0 / sqrt(3.0));
    p = abs(p);
    p -= 2.0 * min(dot(k.xy, p), 0.0) * k.xy;
    p -= float2(clamp(p.x, -k.z * r, k.z * r), r);
    return length(p) * sign(p.y);
}

float2 hextile(inout float2 p)
{
    const float2 sz = float2(1.0, sqrt(3.0));
    const float2 hsz = 0.5 * sz;

    float2 p1 = mod2(p, sz) - hsz;
    float2 p2 = mod2(p - hsz, sz) - hsz;
    float2 p3 = dot(p1, p1) < dot(p2, p2) ? p1 : p2;
    float2 n = ((p3 - p + hsz) / sz);
    p = p3;

    n -= float2(0.5, 0.5);
    return round(n * 2.0) / 2.0;
}

float3 hexTransition(float2 p, float aa, float3 fromColor, float3 toColor, float m)
{
    m = clamp(m, 0.0, 1.0);
    const float hz = HEXTILE_SIZE;
    const float rz = RANDOMNESS;
    
    float2 hp = p;
    hp /= hz;
    
    float2 hn = hextile(hp) * hz * -float2(-1.0, sqrt(3.0));
    float r = hash(hn + 123.4);
    
    const float off = 3.0;
    float fi = smoothstep(0.0, 0.1, m);
    float fo = smoothstep(0.9, 1.0, m);

    float sz = 0.45 * (0.5 + 0.5 * tanh_approx(((rz * r + hn.x + hn.y - off + m * off * 2.0)) * 2.0));
    float hd = (hex(hp, sz) - 0.1 * sz) * hz;
    
    float mm = smoothstep(-aa, aa, -hd);
    mm = lerp(0.0, mm, fi);
    mm = lerp(mm, 1.0, fo);
    
    return lerp(fromColor, toColor, mm);
}

float3 postProcess(float3 col, float2 q)
{
    col = pow(clamp(col, 0.0, 1.0), float3(0.75, 0.75, 0.75));
    col = col * 0.6 + 0.4 * col * col * (3.0 - 2.0 * col);
    col = lerp(col, float3(dot(col, 0.33), dot(col, 0.33), dot(col, 0.33)), -0.4);
    col *= 0.5 + 0.5 * pow(19.0 * q.x * q.y * (1.0 - q.x) * (1.0 - q.y), 0.7);
    return col;
}

// --- メイン処理 ---
float4 main(VS_OUT pin) : SV_TARGET
{
    float2 uv = pin.texcoord;
    
    // 解像度の取得
    float width, height;
    textureMap.GetDimensions(width, height);
    float2 resolution = float2(width, height);

    float2 q = uv;
    float2 p = -1.0 + 2.0 * q;
    p.x *= resolution.x / resolution.y;
    float aa = 2.0 / resolution.y;

    // カラーサンプリング
    float3 sceneCol = textureMap.Sample(samplerStates[LINEAR_BORDER_BLACK], uv).rgb;
    float3 darkCol = float3(0.0, 0.0, 0.0);

    // 開始色(from)と目標色(to)の決定
    float3 fromCol;
    float3 toCol;

    if (isFadeIn)
    {
        // 真っ黒から画面を表示する (FadeIn)
        // transitionProgress = 0.0 で真っ黒、1.0 で完全な画面
        fromCol = darkCol;
        toCol = sceneCol;
    }
    else
    {
        // 画面から真っ黒に落とす (FadeOut)
        // transitionProgress = 0.0 で通常の画面、1.0 で完全な真っ黒
        fromCol = sceneCol;
        toCol = darkCol;
    }

    // トランジションエフェクトの計算
    // ※ 1.25を掛けているのは、ヘキサゴンのタイリングが画面全体を完全に覆い尽くす（あるいは消え去る）までのマージンを確保するためです
    float m = transitionProgress * 1.25;
    float3 col = hexTransition(p, aa, fromCol, toCol, m);
    
    // ポストプロセスの適用
   // col = postProcess(col, q);

    return float4(col, 1.0f);
}