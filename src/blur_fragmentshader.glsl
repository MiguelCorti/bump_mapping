#version 330 core
uniform sampler2D ssao;

in vec2 fragUV; // Coord de textura do fragmento

out float fragColor;

void main()
{
    vec2 texelSize = 1.0/vec2(textureSize(ssao, 0));
    float result = 0.0;
    float blurSamples = 16.0;
    int limit = int(sqrt(blurSamples)/2);
    for(int x=-limit; x<limit; x++)
    {
        for(int y=-limit; y<limit; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssao, fragUV + offset).r;
        }
    }
    fragColor = result/blurSamples;
}
