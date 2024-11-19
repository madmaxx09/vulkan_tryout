#version 450 

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;


layout(push_constant) uniform Push {
	mat2 transform;
	vec2 offset;
	vec3 color;
} push;

void main()
{
	gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0); 
}

//vertexindex s'incremente adéquatement à chaque lancement de cette fonction (aka une fois par vertex)
	//argument 3 == z_value, entre 0 (avant plan) et 1 (arrière plan) aka la "couche" à laquelle ce shader opère 
	//argument 4 == toutes les valeurs de positions sont divisées par ce facteur à la fin jsp encore à quoi ça sert