#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Framework/Light.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Renderer/Renderable/Renderable.hpp"
#include "Engine/Effect/ParticleEmitter.hpp"
class SpriteSheet;
class Clock;


class Mesh;

class Game {
public:
  Game();
  ~Game();
  void loadResources();
  void update(float dSecond);
  void render() const;
  void processInput(float dSecond);
  void beforeFrame();
  void afterFrame();

  void addLight();
protected:

  void updateLight();
  Camera * mCamera;
  Clock* mGameClock;
  S<const Mesh> mShip = nullptr;
  Mesh* mSphere = nullptr;
  const Material* mCoachMat;
  const Material* mShipMat;
  ParticleEmitter* mEmitter;
  Light mLight1, mLight2, mLight3;
  RenderScene* mRenderScene;
  Renderable mRship;
  Renderable mRsphere;
};

extern Game* g_theGame;

