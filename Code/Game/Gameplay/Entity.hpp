#pragma once
#include <string>
#include "Engine/Math/Primitives/vec2.hpp"
class XboxController;
class Entity {
public:
  static constexpr float MAX_SPEED = 2.f;
  static constexpr float MAX_TURN_ANGULAR_SPEED = 180.f;

  Entity(const std::string& type, vec2 position = vec2::zero, float orientation = 0);
  void update(float dSecond);
  void render() const;
  bool isCollidedWith(const Entity& another);
  bool ifCollidedWith(const vec2& point);
  void correctPosition();
public:
  vec2 m_position;
  vec2 m_velocity;
  float rotation = 0;
  float m_orientation = 0;
  float m_angularVelocity = 0;
  XboxController* m_controller;
protected:
//  bool isCollidable(const Tile& tile) const;
  float m_ageSec = 0;
  float m_physicsRadius = .3f;
  float m_cosmaticRadius = .4f;
  bool m_walkable = true;
  bool m_swimmable = false;
  bool m_flyable = false;
};
