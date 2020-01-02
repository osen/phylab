#include "stent.h"

#define RE_PRIMITIVE_TRIANGLE 3
#define RE_PRIMITIVE_SQUARE 4
#define RE_PRIMITIVE_CUBE 14
#define RE_PRIMITIVE_SPHERE 19

struct ReMesh;
struct ReBuffer;

void ReMeshDestroy(ref(ReMesh) ctx);
ref(ReBuffer) ReMeshPositionBuffer(ref(ReMesh) ctx);
ref(ReBuffer) ReMeshNormalBuffer(ref(ReMesh) ctx);
void ReMeshParse(ref(ReMesh) ctx, char *objData);
