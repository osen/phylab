#include "phy.h"

#include <windows.h>

#include <cmath>

int main()
{
  Object a;
  Object b;
  float v = 0;
  Object c;

  a.setPosition(0, 1.5f, 0);
  a.setScale(3, 3, 3);
  a.setType(PHY_SPHERE);
  a.setColor(0, 0, 1);

  while(true)
  {
    v += 0.01f;
    float s = sin(v);
    b.setPosition(2, 5 + s * 3, 2);
    b.setRotation(0, s * 45, 0);
    b.setScale(2, 2 + s * 1, 2);

    Sleep(10);
  }

  return 0;
}
