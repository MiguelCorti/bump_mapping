#version 330 core
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;
uniform sampler2D ssao;

uniform vec3 lightPosition; // Posicao da luz no espaco da camera
//uniform Material material;
uniform vec3 viewPos; // Posicao da camera

in vec2 fragUV; // Coord de textura do fragmento

out vec4 finalColor;

void main()
{
    vec3 fragPos = texture(gPosition, fragUV).rgb;
    vec3 fragNormal = texture(gNormal, fragUV).rgb;
    vec3 textureRGB = texture(gColorSpec, fragUV).rgb;
    float occlusionFactor = texture(ssao, fragUV).r;
    vec3 lightColor = vec3(0.7,0.7,0.9); // hardcoded

    // CALCULO DA LUZ USANDO BLINN PHONG
    vec3 N = fragNormal;
    vec3 L = normalize(lightPosition - fragPos);
    vec3 V = normalize(-fragPos); // ja esta em view space
    vec3 H = normalize(L + V);

    vec3 ambient = textureRGB * 0.2 * occlusionFactor; // fator ambiente considerando oclusao

    float iDif = max(dot(L,N), 0.0);
    vec3 diffuse = iDif * textureRGB * lightColor;

    float iSpec = pow(max(dot(N,H),0.0), 100.0);
    vec3 specular = lightColor * iSpec;

    // MONTANDO A COR FINAL
    finalColor = vec4(ambient + diffuse + specular, 1.0);
}
