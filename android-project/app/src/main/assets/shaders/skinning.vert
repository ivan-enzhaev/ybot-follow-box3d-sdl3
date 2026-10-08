// The #version is injected by createShaderProgram

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aJoints;  // Up to 4 influencing joints per vertex
layout (location = 3) in vec4 aWeights; // Weights corresponding to each joint (must sum to 1.0)
layout (location = 4) in vec3 aNormal;  // Vertex normal for 3D lighting

out vec2 vTexCoord;
out vec3 vFragPos;
out vec3 vNormal;

uniform mat4 uMvpMatrix;
uniform mat4 uModelMatrix;

// Maximum number of joints supported in the skinning palette uniform array
const int MAX_JOINTS = 100;
uniform mat4 uJointMatrices[MAX_JOINTS];

void main()
{
    // Calculate the blended skinning matrix
    mat4 skinMatrix = 
          aWeights.x * uJointMatrices[int(aJoints.x)]
        + aWeights.y * uJointMatrices[int(aJoints.y)]
        + aWeights.z * uJointMatrices[int(aJoints.z)]
        + aWeights.w * uJointMatrices[int(aJoints.w)];

    // Skinned position in local and world space
    vec4 skinnedPos = skinMatrix * vec4(aPosition, 1.0);
    vec4 worldPos = uModelMatrix * skinnedPos;
    vFragPos = worldPos.xyz;

    // Skinned normal rotated to world space
    vec4 skinnedNormal = skinMatrix * vec4(aNormal, 0.0);
    vNormal = normalize(mat3(uModelMatrix) * skinnedNormal.xyz);

    vTexCoord = aTexCoord;
    gl_Position = uMvpMatrix * skinnedPos;
}
