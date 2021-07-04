#version 330 core
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;

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
    float specIntensity = texture(gColorSpec, fragUV).a;

    // CALCULO DA COR
    vec3 ambient = textureRGB * 0.1;

    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPosition - fragPos);

    float iDif = max(dot(L,N), 0.0);
    vec3 diffuse = iDif * textureRGB;

    vec3 V = normalize(viewPos-fragPos);
    vec3 H = normalize(L + V);

    float iSpec = pow(max(dot(N,H),0.0), 100.0);
    vec3 specular = vec3(1.0,1.0,1.0) * iSpec * specIntensity * textureRGB;

    // MONTANDO A COR FINAL
    finalColor = vec4(ambient + diffuse + specular, 1.0);
}
