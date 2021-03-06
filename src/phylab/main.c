#include "TcpSocket.h"
#include "../re/util.h"

#include <re/re.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <stdio.h>

#define VERSION "1.0.4"
#define MOUSE_SENSITIVITY 0.5f;
#define MOVEMENT_SPEED 10;
#define LISTEN_PORT 1987
#define RECV_BUFFER 4096

struct State;

struct Camera
{
  ReVec3 position;
  ReVec3 rotation;
  ref(State) state;
};

struct Object
{
  int id;
  int type;
  ReVec3 position;
  ReVec3 rotation;
  ReVec3 scale;
  ReVec4 color;
  ref(State) state;
};

struct State
{
  SDL_Window *window;
  SDL_GLContext glContext;
  float lastTime;
  float deltaTime;
  ref(ReContext) context;
  ref(ReRenderer) renderer;
  ReVec2 keyboard;
  ReVec2 mouse;
  ReVec2 mouseGrab;
  ref(Camera) camera;
  ref(ReMesh) cube;
  ref(ReMesh) sphere;
  ref(ReMesh) square;
  ref(ReMesh) triangle;
  vector(ref(Object)) objects;
  ref(Object) ground;
  ref(WsTcpSocket) listen;
  ref(WsTcpSocket) client;
  vector(unsigned char) buffer;
  vector(unsigned char) stream;
  ref(sstream) packet;
  vector(ref(sstream)) tokens;
};

ref(State) StateCreate();
void StateDestroy(ref(State) ctx);
void StateClearObjects(ref(State) ctx);
void CameraUpdate(ref(Camera) ctx);
ReMat4 CameraView(ref(Camera) ctx);
ReMat4 CameraProjection(ref(Camera) ctx);
void ObjectRender(ref(Object) obj);
void StateProcessNetwork(ref(State) ctx);

int main()
{
  ref(State) state = NULL;
  ssize_t oi = 0;
  ref(Object) obj = NULL;

  state = StateCreate();

  obj = allocate(Object);
  _(obj).state = state;
  _(obj).color = ReVec4Rgba(1, 0, 0, 1);
  _(obj).scale = ReVec3Xyz(2, 2, 2);
  _(obj).position = ReVec3Xyz(-1, 1, 0);
  _(obj).type = RE_PRIMITIVE_SPHERE;
  vector_push_back(_(state).objects, obj);

  obj = allocate(Object);
  _(obj).state = state;
  _(obj).color = ReVec4Rgba(0, 0, 1, 1);
  _(obj).scale = ReVec3Xyz(2, 2, 2);
  _(obj).position = ReVec3Xyz(2, 1, 1);
  _(obj).rotation = ReVec3Xyz(0, 25, 0);
  _(obj).type = RE_PRIMITIVE_CUBE;
  vector_push_back(_(state).objects, obj);

  int running = 1;
  SDL_Event e = {0};

  while(running)
  {
    while(SDL_PollEvent(&e) != 0)
    {
      if(e.type == SDL_QUIT)
      {
        running = 0;
      }
      else if(e.type == SDL_MOUSEMOTION)
      {
        _(state).mouse = ReVec2Xy(e.motion.x, e.motion.y);
      }
      else if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT)
      {
        _(state).mouse = ReVec2Xy(e.button.x, e.button.y);
        _(state).mouseGrab = _(state).mouse;
      }
      else if(e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT)
      {
        _(state).mouse = ReVec2Xy(e.button.x, e.button.y);
        _(state).mouseGrab = ReVec2Xy(-1, -1);
      }
      else if(e.type == SDL_KEYDOWN)
      {
        if(e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_w)
        {
          _(state).keyboard.y = 1;
        }
        else if(e.key.keysym.sym == SDLK_DOWN || e.key.keysym.sym == SDLK_s)
        {
          _(state).keyboard.y = -1;
        }
        else if(e.key.keysym.sym == SDLK_RIGHT || e.key.keysym.sym == SDLK_d)
        {
          _(state).keyboard.x = 1;
        }
        else if(e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_a)
        {
          _(state).keyboard.x = -1;
        }
        else if(e.key.keysym.sym == SDLK_F12)
        {
          StateClearObjects(state);
        }
      }
      else if(e.type == SDL_KEYUP)
      {
        if(e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_w ||
          e.key.keysym.sym == SDLK_DOWN || e.key.keysym.sym == SDLK_s)
        {
          _(state).keyboard.y = 0;
        }
        else if(e.key.keysym.sym == SDLK_RIGHT || e.key.keysym.sym == SDLK_d ||
          e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_a)
        {
          _(state).keyboard.x = 0;
        }
      }
    }

    float nowTime = SDL_GetTicks();
    float diffTime = nowTime - _(state).lastTime;
    _(state).lastTime = nowTime;
    _(state).deltaTime = diffTime / 1000.0f;

    StateProcessNetwork(state);

    CameraUpdate(_(state).camera);

    ReRendererClear(_(state).renderer, ReVec4Rgba(0.39f, 0.58f, 0.93f, 1.0f));

    for(oi = 0; oi < vector_size(_(state).objects); oi++)
    {
      ObjectRender(vector_at(_(state).objects, oi));
    }

    ObjectRender(_(state).ground);

    SDL_GL_SwapWindow(_(state).window);
  }

  StateDestroy(state);

  return 0;
}

ref(Object) StateObject(ref(State) ctx, int id)
{
  ref(Object) rtn = NULL;
  size_t oi = 0;

  for(oi = 0; oi < vector_size(_(ctx).objects); oi++)
  {
    rtn = vector_at(_(ctx).objects, oi);

    if(_(rtn).id == id)
    {
      return rtn;
    }
  }

  rtn = allocate(Object);
  _(rtn).id = id;
  _(rtn).state = ctx;
  _(rtn).color = ReVec4Rgba(1, 0, 0, 1);
  _(rtn).scale = ReVec3Xyz(1, 1, 1);
  //_(rtn).position = ReVec3Xyz(2, 1, 1);
  //_(rtn).rotation = ReVec3Xyz(0, 25, 0);
  _(rtn).type = RE_PRIMITIVE_CUBE;
  vector_push_back(_(ctx).objects, rtn);

  return rtn;
}

void StateProcessPacket(ref(State) ctx)
{
  vector(ref(sstream)) tokens = NULL;
  ref(sstream) a = NULL;
  ref(Object) object = NULL;

  //printf("[%s]\n", sstream_cstr(_(ctx).packet));
  tokens = _(ctx).tokens;
  SplitStringWhitespace(_(ctx).packet, tokens);

  if(vector_size(tokens) < 1) return;

  a = vector_at(tokens, 0);

  if(strcmp("position", sstream_cstr(a)) == 0)
  {
    if(vector_size(tokens) < 5) return;
    object = StateObject(ctx, atoi(sstream_cstr(vector_at(tokens, 1))));

    _(object).position = ReVec3Xyz(
      atof(sstream_cstr(vector_at(tokens, 2))),
      atof(sstream_cstr(vector_at(tokens, 3))),
      atof(sstream_cstr(vector_at(tokens, 4))));
  }
  else if(strcmp("rotation", sstream_cstr(a)) == 0)
  {
    if(vector_size(tokens) < 5) return;
    object = StateObject(ctx, atoi(sstream_cstr(vector_at(tokens, 1))));

    _(object).rotation = ReVec3Xyz(
      atof(sstream_cstr(vector_at(tokens, 2))),
      atof(sstream_cstr(vector_at(tokens, 3))),
      atof(sstream_cstr(vector_at(tokens, 4))));
  }
  else if(strcmp("scale", sstream_cstr(a)) == 0)
  {
    if(vector_size(tokens) < 5) return;
    object = StateObject(ctx, atoi(sstream_cstr(vector_at(tokens, 1))));

    _(object).scale = ReVec3Xyz(
      atof(sstream_cstr(vector_at(tokens, 2))),
      atof(sstream_cstr(vector_at(tokens, 3))),
      atof(sstream_cstr(vector_at(tokens, 4))));
  }
  else if(strcmp("type", sstream_cstr(a)) == 0)
  {
    if(vector_size(tokens) < 3) return;
    object = StateObject(ctx, atoi(sstream_cstr(vector_at(tokens, 1))));
    _(object).type = atoi(sstream_cstr(vector_at(tokens, 2)));
  }
  else if(strcmp("color", sstream_cstr(a)) == 0)
  {
    if(vector_size(tokens) < 6) return;

    object = StateObject(ctx, atoi(sstream_cstr(vector_at(tokens, 1))));

    _(object).color = ReVec4Rgba(
      atof(sstream_cstr(vector_at(tokens, 2))),
      atof(sstream_cstr(vector_at(tokens, 3))),
      atof(sstream_cstr(vector_at(tokens, 4))),
      atof(sstream_cstr(vector_at(tokens, 5))));
  }
  else if(strcmp("clear", sstream_cstr(a)) == 0)
  {
    StateClearObjects(ctx);
  }
}

void StateProcessNetwork(ref(State) ctx)
{
  ref(WsTcpSocket) socket = NULL;
  size_t ci = 0;
  char ch = 0;
  size_t lnl = 0;

  if(_(ctx).client)
  {
    socket = _(ctx).client;

    if(!WsTcpSocketConnected(socket))
    {
      printf("Client disconnected\n");
      WsTcpSocketClose(socket);
      _(ctx).client = NULL;
      vector_clear(_(ctx).stream);
      return;
    }

    if(!WsTcpSocketReady(socket))
    {
      return;
    }

    //printf("Data received\n");
    vector_resize(_(ctx).buffer, RECV_BUFFER);
    WsTcpSocketRecv(socket, _(ctx).buffer);

    for(ci = 0; ci < vector_size(_(ctx).buffer); ci++)
    {
      vector_push_back(_(ctx).stream, vector_at(_(ctx).buffer, ci));
    }
  }
  else
  {
    socket = _(ctx).listen;

    if(!WsTcpSocketReady(socket))
    {
      return;
    }

    printf("Client connected\n");
    _(ctx).client = WsTcpSocketAccept(socket);
    StateClearObjects(ctx);
  }

  if(vector_size(_(ctx).stream) < 1)
  {
    return;
  }

  sstream_str_cstr(_(ctx).packet, "");

  for(ci = 0; ci < vector_size(_(ctx).stream); ci++)
  {
    ch = vector_at(_(ctx).stream, ci);

    if(ch == '\r')
    {
      // Ignore
    }
    else if(ch == '\n')
    {
      lnl = ci + 1;
      StateProcessPacket(ctx);
      sstream_str_cstr(_(ctx).packet, "");
    }
    else
    {
      sstream_append_char(_(ctx).packet, ch);
    }
  }

  if(lnl)
  {
    vector_erase(_(ctx).stream, 0, lnl);
  }
}

void ObjectRender(ref(Object) ctx)
{
  ReMat4 model = ReMat4Identity();
  ref(ReRenderer) renderer = NULL;
  ref(State) state = NULL;
  ref(ReMesh) mesh = NULL;

  state = _(ctx).state;
  renderer = _(state).renderer;

  model = ReMat4Translate(model, _(ctx).position);
  model = ReMat4RotateY(model, _(ctx).rotation.y);
  model = ReMat4RotateX(model, _(ctx).rotation.x);
  model = ReMat4RotateZ(model, _(ctx).rotation.z);
  model = ReMat4Scale(model, _(ctx).scale);

  mesh = _(state).cube;

  if(_(ctx).type == RE_PRIMITIVE_CUBE)
  {
    mesh = _(state).cube;
  }
  else if(_(ctx).type == RE_PRIMITIVE_SPHERE)
  {
    mesh = _(state).sphere;
  }
  else if(_(ctx).type == RE_PRIMITIVE_SQUARE)
  {
    mesh = _(state).square;
  }
  else if(_(ctx).type == RE_PRIMITIVE_TRIANGLE)
  {
    mesh = _(state).triangle;
  }

  ReRendererSetLighting(renderer, 1);
  ReRendererSetBackfaceCull(renderer, 1);

  if(_(ctx).type == RE_PRIMITIVE_TRIANGLE || _(ctx).type == RE_PRIMITIVE_SQUARE)
  {
    ReRendererSetBackfaceCull(renderer, 0);
  }

  ReRendererSetModel(renderer, model);
  ReRendererSetView(renderer, CameraView(_(state).camera));
  ReRendererSetProjection(renderer, CameraProjection(_(state).camera));
  ReRendererSetColor(renderer, _(ctx).color);
  ReRendererSetMesh(renderer, mesh);
  ReRendererRender(renderer);
}

ref(State) StateCreate()
{
  ref(State) rtn = NULL;
  ref(Camera) cam = NULL;

  if(SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    panic("Failed to initialize window system");
  }

  rtn = allocate(State);

  _(rtn).window = SDL_CreateWindow("PhyLab " VERSION,
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    800, 600,
    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if(!_(rtn).window)
  {
    panic("Failed to create window");
  }

  _(rtn).glContext = SDL_GL_CreateContext(_(rtn).window);

  if(!_(rtn).glContext)
  {
    panic("Failed to create OpenGL context");
  }

  _(rtn).context = ReContextCreate();
  _(rtn).renderer = ReContextCreateRenderer(_(rtn).context);

  _(rtn).sphere = ReContextCreatePrimitive(_(rtn).context, RE_PRIMITIVE_SPHERE);
  _(rtn).cube = ReContextCreatePrimitive(_(rtn).context, RE_PRIMITIVE_CUBE);
  _(rtn).square = ReContextCreatePrimitive(_(rtn).context, RE_PRIMITIVE_SQUARE);
  _(rtn).triangle = ReContextCreatePrimitive(_(rtn).context, RE_PRIMITIVE_TRIANGLE);

  _(rtn).ground = allocate(Object);
  _(_(rtn).ground).state = rtn;
  _(_(rtn).ground).color = ReVec4Rgba(0.3f, 1.0f, 0.3f, 0.75f);
  _(_(rtn).ground).type = RE_PRIMITIVE_SQUARE;
  _(_(rtn).ground).scale = ReVec3Xyz(50, 50, 50);
  _(_(rtn).ground).rotation.x = -90;

  _(rtn).camera = allocate(Camera);
  _(rtn).mouseGrab = ReVec2Xy(-1, -1);
  _(rtn).lastTime = SDL_GetTicks();

  cam = _(rtn).camera;
  _(cam).state = rtn;
  _(cam).position = ReVec3Xyz(0, 10, 10);
  _(cam).rotation = ReVec3Xyz(-45, 0, 0);

  _(rtn).objects = vector_new(ref(Object));

  _(rtn).listen = WsTcpListen(LISTEN_PORT);
  _(rtn).buffer = vector_new(unsigned char);
  _(rtn).stream = vector_new(unsigned char);
  _(rtn).packet = sstream_new();
  _(rtn).tokens = vector_new(ref(sstream));

  if(!_(rtn).listen)
  {
    panic("Failed to open socket [port 1987] for listening");
  }

  return rtn;
}

void StateClearObjects(ref(State) ctx)
{
  size_t oi = 0;

  for(oi = 0; oi < vector_size(_(ctx).objects); oi++)
  {
    release(vector_at(_(ctx).objects, oi));
  }

  vector_clear(_(ctx).objects);
}

void StateDestroy(ref(State) ctx)
{
  WsTcpSocketClose(_(ctx).listen);

  if(_(ctx).client)
  {
    WsTcpSocketClose(_(ctx).client);
  }

  vector_delete(_(ctx).buffer);
  vector_delete(_(ctx).stream);
  sstream_delete(_(ctx).packet);
  vector_sstream_delete(_(ctx).tokens);
  release(_(ctx).camera);
  StateClearObjects(ctx);
  vector_delete(_(ctx).objects);
  release(_(ctx).ground);
  ReMeshDestroy(_(ctx).cube);
  ReMeshDestroy(_(ctx).sphere);
  ReMeshDestroy(_(ctx).square);
  ReMeshDestroy(_(ctx).triangle);

  ReRendererDestroy(_(ctx).renderer);
  ReContextDestroy(_(ctx).context);

  SDL_GL_DeleteContext(_(ctx).glContext);
  SDL_DestroyWindow(_(ctx).window);
  SDL_Quit();

  release(ctx);
}

void CameraUpdate(ref(Camera) ctx)
{
  ref(State) state = NULL;
  ReMat4 m = CameraView(ctx);
  ReVec4 v = {0};
  ReVec2 mouse = {0};
  ReVec2 keyboard = {0};
  ReVec2 mg = {0};
  float speed = 0;

  state = _(ctx).state;
  mg = _(_(ctx).state).mouseGrab;

  if(mg.x != -1)
  {
    SDL_WarpMouseInWindow(_(state).window, mg.x, mg.y);
    mouse = _(state).mouse;
    _(ctx).rotation.x -= (mouse.y - mg.y) * MOUSE_SENSITIVITY;
    _(ctx).rotation.y += (mouse.x - mg.x) * MOUSE_SENSITIVITY;
  }

  keyboard = _(state).keyboard;
  speed = MOVEMENT_SPEED;
  speed *= _(state).deltaTime;

  m = ReMat4Inverse(m);
  m = ReMat4Translate(m, ReVec3Xyz(keyboard.x * speed, 0, keyboard.y * -speed));
  v = ReMat4MulVec4(m, ReVec4Xyzw(0, 0, 0, 1));
  _(ctx).position = ReVec3Xyz(v.x, v.y, v.z);
}

ReMat4 CameraView(ref(Camera) ctx)
{
  ReMat4 rtn = ReMat4Identity();

  rtn = ReMat4Translate(rtn, _(ctx).position);
  rtn = ReMat4RotateY(rtn, _(ctx).rotation.y);
  rtn = ReMat4RotateX(rtn, _(ctx).rotation.x);
  rtn = ReMat4Inverse(rtn);

  return rtn;
}

ReMat4 CameraProjection(ref(Camera) ctx)
{
  int w = 0;
  int h = 0;

  SDL_GetWindowSize(_(_(ctx).state).window, &w, &h);

  if(w < 1) w = 1;
  if(h < 1) h = 1;

  glViewport(0, 0, w, h);

  return ReMat4Perspective(45, (float)w / (float)h, 0.1f, 1000.0f);
}

