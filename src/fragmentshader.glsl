#version 330 core
struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;
uniform sampler2D normalSampler; // Mapa de Normal
uniform sampler2D sampler; // Textura

in vec2 mapCoord; // Coordenadas do Mapa de Normal
in vec3 lightDir; // Direção da Luz em Espaço Tangente
in vec3 viewDir;  // Direção do Olho em Espaço Tangente

out vec3 finalColor; // Cor do Fragmento

vec3 expand (vec3 v)
{
    return v*2.0-1.0;
}

void main()
{
    vec3 ambient = material.ambient;
    //ambient *= texture(sampler, mapCoord).rgb; // Descomentar essa linha para adicionar mapeamento de textura na cor
    vec3 diffuse = vec3(0.0,0.0,0.0);
    vec3 specular = vec3(0.0,0.0,0.0);

    vec3 N = expand(texture(normalSampler, mapCoord).rgb);
    vec3 L = normalize(lightDir);

    float NdotL = dot(N,L);
    float diff = max(NdotL, 0);


    float iSpec = 0.0;
    if( NdotL > 0 )
    {
        diffuse = diff * material.diffuse;
        //diffuse *= texture(sampler, mapCoord).rgb; // Descomentar essa linha para adicionar mapeamento de textura na cor
        vec3 reflected = normalize(reflect(-L, N));
        //vec3 viewer = normalize(-fragPos);
        iSpec = pow(max(dot(reflected,viewDir),0.0), material.shininess);
        specular = iSpec * material.specular;
    }


    finalColor = ambient + diffuse + specular;
}
