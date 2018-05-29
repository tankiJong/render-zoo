#include "Engine/Math/Primitives/vec2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Renderer/glFunctions.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Renderer/Geometry/Mesher.hpp"
#include "Engine/Renderer/Geometry/Mesh.hpp"
#include "Engine/Debug/Draw.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"
#include "Engine/Math/Curves.hpp"
#include "Engine/Renderer/ForwardRendering.hpp"
#include "Engine/Core/Gradient.hpp"
#include "Engine/Debug/Log.hpp"

Game* g_theGame = nullptr;

Transform tt;
Game::Game() {
  loadResources();

  mGameClock = GetMainClock().createChild();

  mCamera = new Camera();
  mCamera->setProjectionPrespective(30.f, 5.f*CLIENT_ASPECT, 5.f, .1f, 100.f);
  mCamera->setColorTarget(g_theRenderer->getDefaultColorTarget());
  mCamera->setDepthStencilTarget(g_theRenderer->getDefaultDepthTarget());
  mCamera->setFlag(CAM_CLEAR_DEPTH | CAM_CLEAR_COLOR | CAM_EFFECT_BLOOM);
  Debug::setRenderer(g_theRenderer);

  Debug::setClock(&GetMainClock());
  Debug::setDuration(5.f);
 
  Debug::setCamera2D(g_theUiCamera);
  Debug::setCamera(mCamera);

  Debug::drawBasis({ 10, 10, 0 }, vec3::right * 100.f, vec3::up * 100.f, vec3::forward * 100.f);
  //Debug::drawText2("Screen space text", 10.f, { 10.f, 50.f });
  //Debug::drawText("world space text", 1.f, vec3::zero, Debug::INF);
//  Debug::drawBasis();
//  Debug::drawGrid();
//  Debug::drawSphere({ 0, 0, 10.f }, 1.f, 20, 20, 15.f);

  mShip = Resource<Mesh>::get("/data/model/scifi_fighter_mk6/scifi_fighter_mk6.obj");
  mCamera->transfrom().localRotation() = vec3::zero;
  mCamera->translate({ 3.f, 3.f, 3.f });
  mCamera->rotate({ -45.f, 135.f, 0.f });
//  mCamera->lookAt({ 3.f, 3.f, 3.f}, vec3::zero);

  vec3 u = vec3::right.cross(vec3::up);


  Mesher ms;

  //ms.begin(DRAW_TRIANGES)
  //  .sphere({ 2.f, 2.f, 0.f }, .5f, 32u, 16u);
  //ms.genNormal().end();

  ms.begin(DRAW_TRIANGES)
    .cube({ 0.f, 1.f, 0.f }, { 1.f, 1.f, 1.f });
  ms.genNormal().end();

  //ms.begin(DRAW_TRIANGES)
  //  .sphere({ 1.f, 1.f, -1.f }, .5f, 32u, 16u);
  //ms.genNormal().end();

  ms.begin(DRAW_TRIANGES)
    .quad(vec3(0,.5f,0), vec3::right, vec3::forward, { 8.f, 8.f});
  ms.genNormal().end();


  mSphere = ms.createMesh();

  mCoachMat = Resource<Material>::get("material/couch").get();
  mShipMat = Resource<Material>::get("material/ship").get();

  mRship.material(Resource<Material>::get("material/ship"));
  mRship.mesh() = mShip.get();
  mRship.transform() = new Transform();

  mRsphere.material(Resource<Material>::get("material/couch"));
  mRsphere.mesh() = mSphere;
//  auto t = new Transform();
//  t->localPosition().y = 3.f;
  mRsphere.transform() = &tt;

  g_theUiCamera->sort = 1;
  mCamera->sort = 0;

  mEmitter = new ParticleEmitter();
  mEmitter->transform.localPosition() = vec3(0, 1, 0);
  mEmitter->setLifeTime(2.f, 6.f);

  Gradient g;
  g.add(Rgba::yellow, 0.f);
  g.add(Rgba::magenta, 0.3f);
  g.add(Rgba::red, 0.5f);

  Rgba start = g.evaluate(0.2f), mid = g.evaluate(0.5f), end = g.evaluate(0.8f);
  mEmitter->updateFunc = [g](particle_t& p, float dsec) {
    float age = p.normalizedAge();
    Rgba tint = g.evaluate(age);
    tint.a = 255u - unsigned char((float)255 * p.normalizedAge());
    p.tint = tint;
    p.size += vec2(dsec*.01f);
  };

  mRenderScene = new RenderScene();
//  mRenderScene->add(mRship);
  mRenderScene->add(mRsphere);
  mRenderScene->add(mEmitter->renderable());
//  mRenderScene->add(mLight1);
  mRenderScene->add(mLight2);
  mRenderScene->add(mLight3);
//  mRenderScene->add(*g_theUiCamera);
  mRenderScene->add(*mCamera);

  mCamera->handlePrePass([this](const Camera& cam) {
    mEmitter->setup(cam);
  });

}

Game::~Game() {
}

void Game::loadResources() {
  Debug::log("resource is loading...", Rgba::red, 10.f);

  Resource<Texture>::define("/image/couch/couch_diffuse.png");
  Resource<Texture>::define("/image/couch/couch_normal.png");
  Resource<Texture>::define("/data/model/scifi_fighter_mk6/SciFi_Fighter-MK6-diffuse.jpg");
  Resource<Texture>::define("/data/model/scifi_fighter_mk6/SciFi_Fighter-MK6-normal.png");
  // Resource<Texture>::define("/image/particle.png");

  Resource<Mesh>::define("/data/model/scifi_fighter_mk6/scifi_fighter_mk6.obj");

  FileSystem::Get().foreach("/data", [](const fs::path& p, auto...) {
    if(p.extension() == ".shader") {
      Resource<Shader>::define(p.generic_string());
      return;
    }
  });
  FileSystem::Get().foreach("/data", [](const fs::path& p, auto...) {
    if (p.extension() == ".mat") {
      Resource<Material>::define(p.generic_string());
      return;
    }
  });
  Debug::log("resource is loaded...", Rgba::green, 15.f);

  // Resource<Shader>::define("/shader/default.shader");
  // Resource<Shader>::define("/shader/default_lit.shader");
  // Resource<Shader>::define("/shader/font.shader");
  // Resource<Shader>::define("/shader/effect_bloom.shader");
  // Resource<Shader>::define("/shader/effect_fog.shader");

  //
  // Resource<Material>::define("/shader/couch.mat");
  // Resource<Material>::define("/shader/ship.mat");
  // Resource<Material>::define("/shader/particle.mat");
}

void Game::update(float dsec) {
  updateLight();
  //mEmitter->emit(10);
  mEmitter->update();
//  static const Debug::DrawHandle* handle = Debug::drawCube(vec3::zero, vec3::one);
//
//  if(mGameClock->total.second > 3.f) {
//    handle->terminate();
//  }
}



void Game::render() const {
  static ForwardRendering fw(g_theRenderer);

  fw.render(*mRenderScene);

  //Debug::log("this is the first log", Rgba::red, 0.f);
  //Debug::log("this is the second log", Rgba::yellow, 0.f);
  Debug::drawNow();

}

void Game::processInput(float dSecond) {
  static int frameCount = 0;
  
  if (frameCount++ < 10) return;
  static vec3 position(10.f, 10.f, 10.f);

  vec2 mouseDelta = g_theInput->mouseDeltaPosition(false);

  vec3 d(mouseDelta.y, mouseDelta.x, 0);

  if(d.magnitude() > 0) {
//    d = d.normalized();
    d.x *= dSecond * .8f;
    d.y *= dSecond * .8f;
  }

  mCamera->rotate({d.x, d.y, 0});
  d = vec3::zero;
  vec3 fwd = mCamera->forward();
  if (g_theInput->isKeyDown('W')) {
    d += mCamera->forward() * dSecond * 5.f;
  }
  if (g_theInput->isKeyDown('S')) {
    d -= mCamera->forward() * dSecond * 5.f;
  }
  if (g_theInput->isKeyDown('A')) {
    d -= mCamera->right() * dSecond * 5.f;
  }
  if (g_theInput->isKeyDown('D')) {
    d += mCamera->right() * dSecond * 5.f;
  }
  mCamera->translate(d);

  if(g_theInput->isKeyJustDown('B')) {
    addLight();
  }

  if(g_theInput->isKeyJustDown(KEYBOARD_F5)) {
    Resource<Shader>::reload();
  }

}

void Game::beforeFrame() {
  g_theRenderer->updateTime((float)mGameClock->frame.second, (float)GetMainClock().frame.second);
}

void Game::afterFrame() {
}

void Game::addLight() {
  Light* l = new Light();
  Rgba color = Hue(getRandomf(0, 360.f));
  l->asSpotLight(.1f, 15.f, 1.f, { 0, 1, 0 }, color);
  l->transform = mCamera->transfrom();
  l->castShadow = true;
  mRenderScene->add(*l);
  Debug::drawCone(l->transform.position(), l->transform.forward(), .5f, 5.f, 10, Debug::INF, true, color);


}

//Camera* createCamera() {
//  Camera* cam = new Camera();
//  cam->transfrom().rotation() = vec3::zero;
//  return cam;
//}
void Game::updateLight() {
  static bool a = true;
  static bool b = true;
  vec3 position3(0, 10.f, 0);
  vec3 position = mCamera->transfrom().position();


//  mLight2.transform.localPosition() = { 0, 5, 0};
//  mLight2.transform.localRotation() = { -89.f, 0, 0 };
    mLight1.transform.localPosition() = position;
  if(a) {
    mLight3.transform = mCamera->transfrom();
    mLight2.transform.localPosition() = mCamera->transfrom().position(); 
  }

  mLight2.transform.localRotation() = { -30.f, 180.f * sinf((float)mGameClock->total.second), 0 };
  if(g_theInput->isKeyJustDown(KEYBOARD_SPACE)) {
    a = !a;
  }
//  mLight3.transform.localPosition() = { cosf(mGameClock->total.second), 4, sinf(mGameClock->total.second) };
  mLight1.asPointLight(.4f, { 1, 0, 0 });
  mLight1.castShadow = false;
  mLight2.castShadow = true;
  mLight3.castShadow = true;
  mLight3.asSpotLight(5.f, 15.f, 1.f, { 0, 1, 0 }, Rgba::red);
  mLight2.asSpotLight(5.f, 15.f, 1.f, { 0, 1, 0 }, Rgba::blue);
//  g_theRenderer->setPointLight(0, position, .4f, { 1, 0, 0 });
//  g_theRenderer->setDirectionalLight(2, position3, mCamera->forward(), .5f, vec3::right, Rgba::blue);
//  Debug::drawPoint(position, 0); 
//  g_theRenderer->setDirectionalLight(0, position, vec3(0, 1, 0).normalized(), .5f, {1, 0, 0});
//  g_theRenderer->setSpotLight(1, position2,mCamera->forward(), 5.f, 5.f, .5f, { 1, 0, 0 }, Rgba::red);
  Debug::drawCone(mLight2.transform.position(),mLight2.transform.forward(), .5f, 5.f, 10, 0, true, Rgba::red);
  Debug::drawCone(mLight3.transform.position(), mLight3.transform.forward(), .5f, 15.f, 10, 0, true, Rgba::blue);
  g_theRenderer->setAmbient(Rgba::white, .2f);
  g_theRenderer->setUnifrom("SPECULAR_AMOUNT", 3.f);
  g_theRenderer->setUnifrom("SPECULAR_POWER", 10.f);
}