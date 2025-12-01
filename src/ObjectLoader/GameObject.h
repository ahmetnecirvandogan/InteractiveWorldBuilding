#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Angel.h"
#include "ObjectLoader.h"
#include "../Core/Shader.h" // Include Shader for Render signature

class GameObject{
public:
    GameObject(ObjectLoader &objectLoader);
    ~GameObject();
    void Move(vec4 pos);
    void SetPosition(vec4 pos);
    void RotateY(float angle);
    void RotateX(float angle);
    void RotateZ(float angle);
    void Rotate(float angle);
    void Scale(float amount);
    vec4 GetPosition();
    void Render(Shader& shader); 
    mat4 objectModelMatrix;
    bool isInPlacement;
    
    // Bounding box methods
    vec3 GetBoundingBoxSize() const;
    float GetWidth() const;  // X dimension
    float GetDepth() const;  // Z dimension
    float GetHeight() const; // Y dimension
    
private:
    vec4 position;
    float angleY,angleZ,angleX;
    float scale;
    ObjectLoader* objectLoader;
    
    void UpdateModelMatrix();
};

#endif // GAME_OBJECT_H
