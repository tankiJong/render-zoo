#ifndef DEFINE_LIGHT_F
#define DEFINE_LIGHT_F

#include "./common.glsl"
#include "./math.glsl"
#define NUM_MAX_LIGHT 8

layout(location = 1) out vec4 bloomColor;
struct light_t {
  vec4 color;

  vec3 attenuation;
  float dotInnerAngle;

  vec3 specAttenuation;
  float dotOuterAngle;

  vec3 position;
  float directionFactor;

  vec3 direction;
  float _pad00;

  mat4 vp;
};

struct light_buffer_t {
  vec4 ambience;
  light_t lights[NUM_MAX_LIGHT];
};

layout(std140, binding = UNIFORM_LIGHT) uniform cLights {
  light_buffer_t lights;
};
layout(binding = UNIFORM_USER_1) uniform sampler2D gTexShadowMap;


float shadowFactor[NUM_MAX_LIGHT];

vec2 ShadowSpread[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

const float ShadowSpreadRatio = 1.f / 500.f;
// 0 means in the shadow
float ShadowTestCore(vec3 worldPosition, mat4 lightVP, float index) {
  vec4 world = vec4(worldPosition, 1.f);
  vec4 clip = lightVP * world;

  vec3 ndc = clip.xyz / clip.w;
  vec3 uvd = ndc * .5f + vec3(.5f);
  uvd.z -= 0.001;
  vec2 uv = uvd.xy;

  if(uv.x > 1.f || uv.y > 1.f || uv.x < 0.f || uv.y < 0.f) return 1.f;
  uv.x = uv.x / float(NUM_MAX_LIGHT) + index * (1.f / float(NUM_MAX_LIGHT));

  float depth = texture( gTexShadowMap, uv).r;
  for(int i = 0; i < 16; i++) {
    float d = texture( gTexShadowMap, uv + ShadowSpread[i]*ShadowSpreadRatio).r;
    depth += ( uvd.z < d) ? 1.f : 0.f;
  }

  depth /= 17.f;

  return depth;
  // return depth;
}

void ShadowTest(vec3 worldPosition) {
  for(int i = 0; i < NUM_MAX_LIGHT; i++) {
    shadowFactor[i] = ShadowTestCore(worldPosition, lights.lights[i].vp, i);
  }
}


/*
              Point             Directional      Spot

dir         lpos - wpos            -ldir          lpos - wpos     - need normalize
*/
vec3 IncidenceDirection(light_t from, vec3 toPosition) {
  vec3 dir = from.direction;
  vec3 pointLightDir =  toPosition - from.position;

  return normalize(mix(pointLightDir, dir, from.directionFactor));
}

vec3 Ambient(vec4 ambience) {
  return ambience.xyz * ambience.w;
}

float Attenuation(float intensity, float dis, vec3 factor) {
  float atten = intensity / ( factor.x 
      + factor.y * dis 
      + factor.z * dis * dis);

  return clamp(atten, 0.f, 1.f);
}

float LightPower(light_t light, vec3 surfacePosition) {
  vec3 inDirection = IncidenceDirection(light, surfacePosition);

  float dotAngle = dot(inDirection, normalize(surfacePosition - light.position));
  return smoothstep(light.dotOuterAngle, light.dotInnerAngle, dotAngle);
}

vec3 Diffuse(light_t light, vec3 surfacePosition, vec3 surfaceNormal) {
  vec3 inDirection = IncidenceDirection(light, surfacePosition);
  float dot3 = dot(-inDirection, surfaceNormal);

  float lightPower = LightPower(light, surfacePosition);
  
  float dis = distance(light.position, surfacePosition);
  float atten = Attenuation(light.color.w, dis, light.attenuation);
  return lightPower * dot3 * atten * light.color.xyz;
}

vec3 Diffuse(light_buffer_t lights,
             vec3 surfacePosition, vec3 surfaceNormal) {
  // dot3 diffuse
  vec3 diffuse = vec3(0);

  for(int i = 0; i < NUM_MAX_LIGHT; i++) {
    vec3 diffu = Diffuse(lights.lights[i], surfacePosition, surfaceNormal);
    diffuse += diffu * shadowFactor[i];
    // diffuse += mix(diffu, vec3(0),
    //            ShadowTest(surfacePosition, lights.lights[i].vp, i));
  }

  return clamp(diffuse, vec3(0), vec3(1));
}

vec3 Specular(light_t light, vec3 surfacePosition, vec3 surfaceNormal, float shininesss, float smoothness, vec3 eyeDir) {
  vec3 inDirection = IncidenceDirection(light, surfacePosition);
  vec3 r = reflect(inDirection, surfaceNormal);

  
  float factor = max(0.f, dot(eyeDir, r));
  factor = shininesss * pow(factor, smoothness);
  factor *= LightPower(light, surfacePosition);

  float dis = distance(light.position, surfacePosition);
  float atten = Attenuation(light.color.w, dis, light.attenuation);

  return factor * atten * light.color.xyz;
}

vec3 Specular(light_buffer_t lights, vec3 surfacePosition, vec3 surfaceNormal, float shininesss, float smoothness, vec3 eyeDir) {
  vec3 specular = vec3(0);

  for(int i = 0; i < NUM_MAX_LIGHT; i++) {
    vec3 spec = Specular(lights.lights[i], surfacePosition, surfaceNormal, shininesss, smoothness, eyeDir);
    specular += spec * shadowFactor[i];
    // specular -= mix(spec, vec3(0), 
    //                 ShadowTest(surfacePosition, lights.lights[i].vp, i));

  }

  return specular;
}
// vec4 ShadowTestCore2(vec3 worldPosition, mat4 lightVP, float index) {
//   vec4 world = vec4(worldPosition, 1.f);
//   vec4 clip = lightVP * world;

//   vec3 ndc = clip.xyz / clip.w;
//   vec3 uvd = ndc * .5f + vec3(.5f);
//   vec2 uv = uvd.xy;

//   // if(uv.x > 1.f || uv.y > 1.f || uv.x < 0.f || uv.y < 0.f) return 1.f;
//   uv.x = uv.x / float(NUM_MAX_LIGHT) + index * (1.f / float(NUM_MAX_LIGHT));
//   return texture( gTexShadowMap, uv );
//   // return vec4(uvd.xy, 0 ,1);
//   // return ( uvd.z < depth + 0.001) ? 1.f : 0f;
//   // return depth;
// }

vec4 PhongLighting(light_buffer_t lights, 
                   vec3 surfacePosition, vec3 surfaceNormal, float shininesss, float smoothness, vec4 surfaceColor,
                   vec3 eyeDir) {
  // float d = ShadowTestCore2(surfacePosition, lights.lights[0].vp, 0);
  // return vec4(d,0,0 ,1);
  // return ShadowTestCore2(surfacePosition, lights.lights[0].vp, 0);
  ShadowTest(surfacePosition);
  #ifndef _disable_diffuse
  vec3 diffuse = Diffuse(lights, surfacePosition, surfaceNormal);
  #else
  vec3 diffuse = vec3(0);
  #endif
  
  #ifndef _disable_specular
  vec3 specular = Specular(lights, surfacePosition, surfaceNormal, shininesss, smoothness, eyeDir);
  #else
  vec3 specular = vec3(0);
  #endif

  #ifndef _disable_ambient
  vec3 surfaceLight = Ambient(lights.ambience) + diffuse;
  #else
  vec3 surfaceLight = diffuse;
  #endif

  clamp(surfaceLight, vec3(0), vec3(1));

  #ifdef _d_light_only
  vec4 finalColor = vec4(surfaceLight, 1) * vec4(1) + vec4(specular, 0);
  #else
  vec4 finalColor = vec4(surfaceLight, 1) * surfaceColor + vec4(specular, 0);
  #endif

  bloomColor = vec4(specular, 1.f);
  return clamp(finalColor, vec4(0), vec4(1));
}


#endif