#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 squareColor; // La couleur de base (blanc ou noir)
uniform vec3 uLightDir;   // Direction de la lumière
uniform vec3 uViewPos;    // Position de la caméra

void main() {
    // 1. Ambiant (Lumière minimale pour ne pas avoir de noir absolu)
    vec3 ambient = 0.3 * vec3(1.0);

    // 2. Diffus (Lumière principale basée sur l'angle de la face)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    // 3. Spéculaire (Le reflet brillant sur les bords)
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.5 * spec * vec3(1.0);

    // Résultat final : on combine tout et on applique à la couleur de la case
    vec3 result = (ambient + diffuse + specular) * squareColor;
    FragColor = vec4(result, 1.0);
}