#define POINT 0 /* Point wrap filtering (Default) */
#define LINEAR 3 /* Linear wrap filtering (Default) */
#define ANISOTROPIC 6 /* Anisotropic wrap filtering (Default) */

#define POINT_WRAP 0
#define POINT_BORDER 1
#define POINT_CLAMP 2
#define LINEAR_WRAP 3
#define LINEAR_BORDER 4
#define LINEAR_CLAMP 5
#define ANISOTROPIC_WRAP 6
#define ANISOTROPIC_BORDER 7
#define ANISOTROPIC_CLAMP 8

#define LINEAR_BORDER_BLACK 9
#define LINEAR_BORDER_WHITE 10
SamplerState samplerStates[11] : register(s0);