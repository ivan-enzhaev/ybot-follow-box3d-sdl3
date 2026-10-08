// The #version and precision qualifiers are injected by createShaderProgram

in vec2 vTexCoord;
in vec3 vFragPos;
in vec3 vNormal;

uniform sampler2D ourTexture;
uniform vec3 uViewPos;
uniform vec3 uLightDir;

out vec4 fragColor;

void main()
{
    vec4 texColor = texture(ourTexture, vTexCoord);
    if (texColor.a < 0.1)
        discard;

    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);

    // Ambient lighting (soft sky fill)
    vec3 ambient = vec3(0.35, 0.38, 0.44);

    // Diffuse sunlight (warm direct key light)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 sunColor = vec3(1.0, 0.98, 0.92);
    vec3 diffuse = diff * sunColor;

    // Specular highlights (Blinn-Phong gloss/shine)
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.4) * spec * sunColor;

    // Fresnel rim backlight (accents robot contours and joints)
    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim = pow(rim, 3.0) * 0.35;
    vec3 rimLight = vec3(0.4, 0.6, 1.0) * rim;

    vec3 finalRgb = (ambient + diffuse) * texColor.rgb + specular + rimLight;
    fragColor = vec4(finalRgb, texColor.a);
}
