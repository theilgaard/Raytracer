#ifndef AREALIGHT_H_
#define AREALIGHT_H_

#include <vector>
#include "Vector3.h"
#include "Ray.h"

class Scene;

class AreaLight
{
public:
    void setPosition(const Vector3& v)  {center = v;}
    void setAxisFixed(bool x, bool y, bool z) {xfixed = x; yfixed = y; zfixed = z;}
    void setNormal(const Vector3& v)  	{n = v;}
    void setColor(const Vector3& v)     {m_color = v;}
    void setWattage(float f)            {m_wattage = f;}

    float wattage() const               {return m_wattage;}
    const Vector3 & color() const       {return m_color;}

    void preCalc() {} // use this if you need to

    const Vector3& getCenter() const     {return center;}
    virtual Vector3 generateRandomPosition();

protected:
    Vector3 center;
    Vector3 n; //normal
    Vector3 m_color;
    float m_wattage;
    bool xfixed, yfixed, zfixed;
};



#endif /* AREALIGHT_H_ */
