uniform sampler2D texture;
uniform vec2 ballPos;
uniform vec2 resolution;
uniform float radius;


float Blob(vec2 position,vec2 point, float radius)
{
	float temp = pow(position.x - point.x, 2.0) + pow(position.y - point.y, 2.0);
	float result = texture2D(texture, gl_TexCoord[0].xy);
	if (temp < pow(radius, 2.0))
	{		
		float distance = sqrt(pow(position.x - point.x, 2.0) + pow(position.y - point.y, 2.0)) / radius;		
		result = pow((1.0 - pow(distance, 2.0)), 2.0);
	}
	return result;
}

void main()
{
	//float PI = 3.141516;
	//vec2 position = gl_FragCoord.xy / resolution.xy;

	float blobValue = texture2D(texture, gl_TexCoord[0].xy);
	//blobValue += Blob(position, ballPos, radius);
	
	vec4 color = vec4(0.0, blobValue / 2.0, blobValue, 1.0);
	color = floor(color / 0.3) * 0.5;
	gl_FragColor = color;
}