#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

struct DirectionalLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define POINT_LIGHTS_COUNT 4

uniform Material material;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[POINT_LIGHTS_COUNT];
uniform SpotLight spotLight;

uniform vec3 viewPos;
uniform float time;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDirection);
vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDirection);

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDirection = normalize(viewPos - FragPos);

    vec3 result = CalculateDirectionalLight(directionalLight, normal, viewDirection);
    for (int i = 0; i < POINT_LIGHTS_COUNT; ++i)
    {
        result += CalculatePointLight(pointLights[i], normal, FragPos, viewDirection);
    }

    result += CalculateSpotLight(spotLight, normal, FragPos, viewDirection);

    FragColor = vec4(result, 1.0f);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
    vec3 lightDir = normalize(-light.direction);

    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0f), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;

    // emission
    vec3 showEmission = step(vec3(1.0f), vec3(1.0f) - texture(material.specular, TexCoord).rgb);
    vec3 emission = texture(material.emission, TexCoord + vec2(0.0f, time)).rgb * showEmission;

    return (ambient + diffuse + specular + emission);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDirection)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;
    ambient *= attenuation;

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;
    diffuse *= attenuation;

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0f), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;
    specular *= attenuation;

    // emission
    vec3 showEmission = step(vec3(1.0f), vec3(1.0f) - texture(material.specular, TexCoord).rgb);
    vec3 emission = texture(material.emission, TexCoord + vec2(0.0f, time)).rgb * showEmission;
    emission *= attenuation;

    return (ambient + diffuse + specular + emission);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDirection)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // spotlight soft edges
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);

    // calculate attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;
    ambient *= intensity * attenuation;

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;
    diffuse *= intensity * attenuation;

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0f), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;
    specular *= intensity * attenuation;

    // emission
    vec3 showEmission = step(vec3(1.0f), vec3(1.0f) - texture(material.specular, TexCoord).rgb);
    vec3 emission = texture(material.emission, TexCoord + vec2(0.0f, time)).rgb * showEmission;
    emission *= intensity * attenuation;

    return (ambient + diffuse + specular + emission);
}