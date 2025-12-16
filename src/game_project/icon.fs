#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D iconTexture;
uniform vec3 tintColor;
uniform float alpha;

void main()
{
    vec4 texColor = texture(iconTexture, TexCoords);
    FragColor = vec4(texColor.rgb * tintColor, texColor.a * alpha);
}
