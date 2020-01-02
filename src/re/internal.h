#include "stent.h"
#include "mathutil.h"

#include <GL/glew.h>

struct ReContext
{
  int todo;
};

struct ReBuffer
{
  GLuint id;
  int type;
  int dirty;
  vector(float) data;
};

GLuint _ReBufferId(ref(ReBuffer) ctx);
int _ReBufferType(ref(ReBuffer) ctx);

struct ReMesh
{
  ref(ReContext) context;
  ref(ReBuffer) position;
  ref(ReBuffer) normal;
};

struct ReShader
{
  GLuint id;
  GLint colorLoc;
  GLint modelLoc;
  GLint viewLoc;
  GLint projectionLoc;
  GLint normalLoc;
  GLint enableLightingLoc;
};

ref(ReShader) _ReShaderCreate(ref(ReContext) context);
void _ReShaderDestroy(ref(ReShader) ctx);
GLuint _ReShaderId(ref(ReShader) ctx);
GLint _ReShaderColorLoc(ref(ReShader) ctx);
GLint _ReShaderModelLoc(ref(ReShader) ctx);
GLint _ReShaderViewLoc(ref(ReShader) ctx);
GLint _ReShaderProjectionLoc(ref(ReShader) ctx);
GLint _ReShaderNormalLoc(ref(ReShader) ctx);
GLint _ReShaderEnableLightingLoc(ref(ReShader) ctx);

struct ReRenderer
{
  ref(ReShader) shader;
  ref(ReBuffer) position;
  ref(ReBuffer) normal;
  int depthTest;
  int backfaceCull;
  int blend;
  int lighting;
  struct ReVec4 color;
  struct ReMat4 model;
  struct ReMat4 view;
  struct ReMat4 projection;
};

void _RePollForError();
