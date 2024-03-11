#version 330 core

in vec2 vtex;
in vec3 vnorm;
in vec3 fpos;

uniform sampler2D text;
uniform vec3 light_pos;
uniform vec3 viewer_pos;

out vec4 outColor;

vec3 ambient(vec3 light_color, vec3 objColor) {
    float ambient = 0.7;

    vec3 ambientColor = ambient * light_color * objColor;
    return ambientColor;
}

vec3 diffuse(vec3 light_color, vec3 normal, vec3 light_position, vec3 fragment_position) {
    vec3 normalized_normal = normalize(normal);
    vec3 light_direction = normalize(light_position - fragment_position);
    
    // Can't have negative light, so clamp dot product to 0 minimum
    vec3 diffuse = max(dot(normalized_normal, light_direction), 0.0) * light_color;
    return diffuse;
}

vec3 specular(vec3 view_pos, vec3 frag_pos, vec3 light_position, vec3 normal, vec3 light_color) {
    float strength = 0.6;
    int shininess = 16;

    vec3 light_dir = normalize(light_position - frag_pos);
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);

    // Once again, clamp dot product to a minimum value of 0
    vec3 specular = light_color * strength * pow(max(dot(view_dir, reflect_dir), 0.0), shininess);  
    return specular;
}

void main()
{
    // Since all objects have textures, the base "color" of the objects is white
    vec3 defaultColor = vec3(1.0,1.0,1.0);
    // We're just using white light for everything
    vec3 lightColor = vec3(0.7,0.7,0.7);

    // Calculate ambient and diffuse components
    vec3 ambient_color = ambient(lightColor, defaultColor);
    vec3 diffuse_color = diffuse(lightColor, vnorm, light_pos, fpos);
    vec3 specular_color = specular(viewer_pos, fpos, light_pos, vnorm, lightColor);
    vec3 compound_color = (ambient_color + diffuse_color + specular_color) * defaultColor;

    // Add texture and vertex color
    outColor = texture(text, vtex) * vec4(compound_color,1.0);

}