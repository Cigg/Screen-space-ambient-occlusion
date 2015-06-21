#ifndef CONTROLS_H
#define CONTROLS_H

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getPosition();
int isSSAOEnabled();
int isTexturesEnabled();

#endif