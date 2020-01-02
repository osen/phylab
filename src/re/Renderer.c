#include "Renderer.h"
#include "internal.h"
#include "mathutil.h"
#include "Buffer.h"
#include "Mesh.h"

void ReRendererDestroy(ref(ReRenderer) ctx)
{
  _ReShaderDestroy(_(ctx).shader);
  release(ctx);
}

void ReRendererClear(ref(ReRenderer) ctx, struct ReVec4 color)
{
  glClearColor(color.x, color.y, color.z, color.w);
  _RePollForError();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  _RePollForError();
}

void ReRendererSetPositionBuffer(ref(ReRenderer) ctx, ref(ReBuffer) buffer)
{
  _(ctx).position = buffer;
}

void ReRendererSetNormalBuffer(ref(ReRenderer) ctx, ref(ReBuffer) buffer)
{
  _(ctx).normal = buffer;
}

void ReRendererSetColor(ref(ReRenderer) ctx, struct ReVec4 col)
{
  _(ctx).color = col;
}

void ReRendererSetMesh(ref(ReRenderer) ctx, ref(ReMesh) mesh)
{
  _(ctx).position = ReMeshPositionBuffer(mesh);
  _(ctx).normal = ReMeshNormalBuffer(mesh);
}

void ReRendererRender(ref(ReRenderer) ctx)
{
  if(_(ctx).blend)
  {
    glEnable(GL_BLEND);
    _RePollForError();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _RePollForError();
  }

  if(_(ctx).depthTest)
  {
    glEnable(GL_DEPTH_TEST);
    _RePollForError();
  }

  if(_(ctx).backfaceCull)
  {
    glEnable(GL_CULL_FACE);
    _RePollForError();
  }

  glUseProgram(_ReShaderId(_(ctx).shader));
  _RePollForError();

  if(!_(ctx).position)
  {
    panic("No position buffer set");
  }

  glBindBuffer(GL_ARRAY_BUFFER, _ReBufferId(_(ctx).position));
  _RePollForError();

  glVertexAttribPointer(0, _ReBufferType(_(ctx).position), GL_FLOAT, GL_FALSE, 0, 0);
  _RePollForError();
  glEnableVertexAttribArray(0);
  _RePollForError();

  if(!_(ctx).normal)
  {
    if(_(ctx).lighting)
    {
      panic("Lighting enabled but no texture coordinates provided");
    }
  }
  else
  {
    glBindBuffer(GL_ARRAY_BUFFER, _ReBufferId(_(ctx).normal));
    _RePollForError();

    glVertexAttribPointer(1, _ReBufferType(_(ctx).normal), GL_FLOAT, GL_FALSE, 0, 0);
    _RePollForError();
    glEnableVertexAttribArray(1);
    _RePollForError();
  }

  glUniform1i(_ReShaderEnableLightingLoc(_(ctx).shader), _(ctx).lighting);
  _RePollForError();

  glUniform4f(_ReShaderColorLoc(_(ctx).shader),
    _(ctx).color.x,
    _(ctx).color.y,
    _(ctx).color.z,
    _(ctx).color.w);
  _RePollForError();

  glUniformMatrix4fv(_ReShaderProjectionLoc(_(ctx).shader),
    1, GL_FALSE, (float *)_(ctx).projection.m);
  _RePollForError();

  glUniformMatrix4fv(_ReShaderViewLoc(_(ctx).shader),
    1, GL_FALSE, (float *)_(ctx).view.m);
  _RePollForError();

  glUniformMatrix4fv(_ReShaderModelLoc(_(ctx).shader),
    1, GL_FALSE, (float *)_(ctx).model.m);
  _RePollForError();

  glUniformMatrix4fv(_ReShaderNormalLoc(_(ctx).shader),
    1, GL_FALSE, (float *)ReMat4Transpose(ReMat4Inverse(_(ctx).model)).m);
  _RePollForError();

  glDrawArrays(GL_TRIANGLES, 0, ReBufferSize(_(ctx).position));
  _RePollForError();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  _RePollForError();

  glDisable(GL_BLEND);
  _RePollForError();
  glDisable(GL_DEPTH_TEST);
  _RePollForError();
  glDisable(GL_CULL_FACE);
  _RePollForError();

  glUseProgram(0);
  _RePollForError();
}

void ReRendererSetBackfaceCull(ref(ReRenderer) ctx, int enabled)
{
  _(ctx).backfaceCull = enabled;
}

void ReRendererSetBlend(ref(ReRenderer) ctx, int enabled)
{
  _(ctx).blend = enabled;
}

void ReRendererSetDepthTest(ref(ReRenderer) ctx, int enabled)
{
  _(ctx).depthTest = enabled;
}

void ReRendererSetLighting(ref(ReRenderer) ctx, int enabled)
{
  _(ctx).lighting = enabled;
}

void ReRendererSetModel(ref(ReRenderer) ctx, struct ReMat4 model)
{
  _(ctx).model = model;
}

void ReRendererSetView(ref(ReRenderer) ctx, struct ReMat4 view)
{
  _(ctx).view = view;
}

void ReRendererSetProjection(ref(ReRenderer) ctx, struct ReMat4 projection)
{
  _(ctx).projection = projection;
}

