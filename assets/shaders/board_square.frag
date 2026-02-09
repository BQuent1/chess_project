#version 330 core
out vec4 FragColor; // La couleur finale qui sort

uniform vec3 squareColor; // La couleur qu'on envoie depuis le C++ (noir ou blanc)

void main()
{
    FragColor = vec4(squareColor, 1.0); // Alpha à 1.0 (opaque)
}