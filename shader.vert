#version 410

layout(location = 0) in vec3 position;

// Data from CPU
uniform mat4 MVP; // ModelViewProjection Matrix
uniform mat4 MV; // ModelView idMVPMatrix
uniform vec4 cameraPosition;
uniform float heightFactor;

// Texture-related data
uniform sampler2D rgbTexture;
uniform sampler2D grayTexture;
uniform int widthTexture;
uniform int heightTexture;

uniform vec4 lightPosition;

// Output to Fragment Shader
out vec2 textureCoordinate; // For texture-color
out vec3 vertexNormal; // For Lighting computation
out vec3 ToLightVector; // Vector from Vertex to Light;
out vec3 ToCameraVector; // Vector from Vertex to Camera;


void main()
{

    // get texture value, compute height
    textureCoordinate = vec2( widthTexture-position.x/widthTexture, heightTexture-position.z/heightTexture );
    vec4 textureColor = texture(grayTexture, textureCoordinate);
    float y = textureColor.x ;
    vec4 v = vec4(position.x, heightFactor * y,position.z, 1);


    // compute normal vector using also the heights of neighbor vertices

    vec2 texCoord;
    vec4 texColor;
    vec4 nb1 = vec4(0,0,0,1);
    vec4 nb2 = vec4(0,0,0,1);
    vec4 nb4 = vec4(0,0,0,1);
    vec4 nb6 = vec4(0,0,0,1);
    vec4 nb5 = vec4(0,0,0,1);
    vec4 nb3 = vec4(0,0,0,1);

    nb1 = vec4(v.x - 1, 0, v.z - 1, 1);
    texCoord = vec2(widthTexture - nb1.x / widthTexture, heightTexture - nb1.z / heightTexture);
    texColor = texture(grayTexture, texCoord);
    nb1.y = heightFactor * texColor.x;

    nb2 = vec4(v.x - 1, 0, v.z, 1);
    texCoord = vec2(widthTexture - nb2.x / widthTexture, heightTexture - nb2.z / heightTexture);
    texColor = texture(grayTexture, texCoord);
    nb2.y = heightFactor * texColor.x;

    nb3 = vec4(v.x, 0, v.z - 1, 1);
    texCoord = vec2(widthTexture - nb3.x / widthTexture, heightTexture - nb3.z / heightTexture);
    texColor = texture(grayTexture, texCoord);
    nb3.y = heightFactor * texColor.x ;

    nb4 = vec4(v.x, 0, v.z + 1, 1);
    texCoord = vec2(widthTexture - nb4.x / widthTexture, heightTexture - nb4.z / heightTexture);
    texColor = texture(grayTexture, texCoord);
    nb4.y = heightFactor * texColor.x ;

    nb5 = vec4(v.x + 1, 0, v.z, 1);
    texCoord = vec2(widthTexture - nb5.x / widthTexture, heightTexture - nb5.z / heightTexture);
    texColor = texture(grayTexture, texCoord);
    nb5.y = heightFactor * texColor.x;

    nb6 = vec4(v.x + 1, 0, v.z + 1, 1);
    texCoord = vec2(widthTexture - nb6.x / widthTexture, heightTexture - nb6.z / heightTexture);
    texColor = texture(grayTexture, texCoord);
    nb6.y = heightFactor * texColor.x;

    vec3 norm = vec3(0,0,0);
    norm += cross(vec3(nb1-v), vec3(nb2-v));
    norm += cross(vec3(nb2-v), vec3(nb4-v));
    norm += cross(vec3(nb4-v), vec3(nb6-v));
    norm += cross(vec3(nb6-v), vec3(nb5-v));
    norm += cross(vec3(nb5-v), vec3(nb3-v));
    norm += cross(vec3(nb3-v), vec3(nb1-v));

    norm = normalize(norm);
    vertexNormal = normalize(vec3( inverse(transpose(MV)) * vec4(norm,0)));


    // compute toLight vector & toCamera vector vertex coordinate in VCS
    vec3 pos = vec3(v.x,v.y,v.z);
    vec3 pos2 = vec3(MV * vec4(pos,1.0));
    //ToLightVector = normalize(vec3(widthTexture/2.0, 100, heightTexture/2.0) - pos2);
    ToLightVector = normalize(vec3(MV*vec4(lightPosition.xyz - v.xyz ,0)));
    ToCameraVector = normalize(vec3(MV * vec4(cameraPosition.x - v.x, cameraPosition.y - v.y, cameraPosition.z - v.z, 0)));


    // set gl_Position variable correctly to give the transformed vertex position
    gl_Position = MVP * v;
}
