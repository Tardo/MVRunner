// Original Shader by https://ziyadbarakat.wordpress.com/tutorials/graphics/chromatic-aberration-shader/
 
uniform vec3 offc;
uniform sampler2D texture;
 
void main()
{
    //access the colors of the texture at an offset
    float redValue = texture2D(texture, gl_TexCoord[0].xy + offc.r).r;
    float greenValue = texture2D(texture, gl_TexCoord[0].xy + offc.g).g;
    float blueValue = texture2D(texture, gl_TexCoord[0].xy + offc.b).b;
    float alphaValue = texture2D(texture, gl_TexCoord[0].xy).a;
     
    //make a new color from the offset colors
    gl_FragColor = gl_Color * vec4(redValue, greenValue, blueValue, alphaValue);
}
