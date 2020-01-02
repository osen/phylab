#include "stent.h"

struct ReRenderer;
struct ReBuffer;
struct ReVec4;
struct ReMat4;
struct ReMesh;

void ReRendererDestroy(ref(ReRenderer) ctx);
void ReRendererClear(ref(ReRenderer) ctx, struct ReVec4 color);

void ReRendererSetMesh(ref(ReRenderer) ctx, ref(ReMesh) mesh);
void ReRendererSetPositionBuffer(ref(ReRenderer) ctx, ref(ReBuffer) buffer);
void ReRendererSetNormalBuffer(ref(ReRenderer) ctx, ref(ReBuffer) buffer);

void ReRendererSetColor(ref(ReRenderer) ctx, struct ReVec4 col);
void ReRendererSetModel(ref(ReRenderer) ctx, struct ReMat4 model);
void ReRendererSetView(ref(ReRenderer) ctx, struct ReMat4 view);
void ReRendererSetProjection(ref(ReRenderer) ctx, struct ReMat4 projection);
void ReRendererRender(ref(ReRenderer) ctx);
void ReRendererSetBackfaceCull(ref(ReRenderer) ctx, int enabled);
void ReRendererSetBlend(ref(ReRenderer) ctx, int enabled);
void ReRendererSetDepthTest(ref(ReRenderer) ctx, int enabled);
void ReRendererSetLighting(ref(ReRenderer) ctx, int enabled);
