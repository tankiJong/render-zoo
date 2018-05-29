#pragma once

class Game;
class App {
public:
  App();
  ~App();
  void afterFrame();
  void beforeFrame();
  void update();
  void render() const;
  inline unsigned char isQuitting() { return m_isQuitting; }
  inline void setQuitFlag(unsigned char isQuitting) { m_isQuitting = isQuitting; }
private:
  Game * m_game;
  unsigned char    m_lowSpeed = 0;
  unsigned char    m_isPaused = 0;
  unsigned char    m_isQuitting = 0;
};


extern App* g_theApp;