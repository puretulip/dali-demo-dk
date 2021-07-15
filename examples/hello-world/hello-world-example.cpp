/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <GLES2/gl2.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali-toolkit/devel-api/controls/table-view/table-view.h>
#include <dali-toolkit/public-api/controls/gl-view/gl-view.h>
#include <dali/devel-api/common/stage.h>
#include <dali/devel-api/events/touch-point.h>
#include <dali/public-api/signals/callback.h>
#include <dlog.h>
#include <iostream>

using namespace Dali;
using Dali::Toolkit::GlView;
using Dali::Toolkit::TableView;

class HelloWorldController : public ConnectionTracker
{
public:
  HelloWorldController(Application& application)
  : mApplication(application)
  {
    // Connect to the Application's Init signal
    mApplication.InitSignal().Connect(this, &HelloWorldController::Create);
  }

  ~HelloWorldController() = default; // Nothing to do in destructor

  // The Init signal is received once (only) during the Application lifetime
  void Create(Application& application)
  {
    // Get a handle to the window
    Window window = application.GetWindow();
    window.SetTransparency(true);
    // window.SetBackgroundColor(Color::WHITE);
    window.SetBackgroundColor(Color::TRANSPARENT);
    window.AddAvailableOrientation(Dali::WindowOrientation::PORTRAIT);
    window.AddAvailableOrientation(Dali::WindowOrientation::LANDSCAPE);
    window.AddAvailableOrientation(Dali::WindowOrientation::PORTRAIT_INVERSE);
    window.AddAvailableOrientation(Dali::WindowOrientation::LANDSCAPE_INVERSE);

    window.Show();

    TableView page = TableView::New(1, 2);
    page.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER);
    page.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    page.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS);

    window.Add(page);

    mGlView = GlView::New(true);
    // mGlView.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_LEFT);
    mGlView.SetResizePolicy(Dali::ResizePolicy::FILL_TO_PARENT, Dali::Dimension::ALL_DIMENSIONS);
    // mGlView.SetProperty(Actor::Property::SIZE, Vector2(360.0f, 360.0f));

    mGlView.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER);
    mGlView.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    mGlView.SetRenderingMode(GlView::RenderingMode::CONTINUOUS);
    // mGlView.SetRenderingMode(GlView::RenderingMode::ON_DEMAND);

    mGlView.RegisterGlCallback(Dali::MakeCallback(this, &HelloWorldController::initialize_gl),
                               Dali::MakeCallback(this, &HelloWorldController::renderFrame_gl),
                               Dali::MakeCallback(this, &HelloWorldController::terminate_gl));

    bool choose = false;
    choose      = mGlView.SetEglConfig(true, true, 0, GlView::GlesVersion::VERSION_2_0);
    if(!choose)
    {
      printf("choose error\n");
    }
    mGlView.SetResizeCallback(Dali::MakeCallback(this, &HelloWorldController::OnResize));

    // Respond to a touch anywhere on the window
    // window.GetRootLayer().TouchedSignal().Connect(this, &HelloWorldController::OnTouch);
    // window.KeyEventSignal().Connect(this, &HelloWorldController::OnKeyEvent);

    // window.Add(mGlView);
    page.AddChild(mGlView, TableView::CellPosition(0, 0));

    Toolkit::PushButton button = Toolkit::PushButton::New();
    button.SetProperty(Toolkit::Button::Property::LABEL, "on/off");
    button.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_CENTER);
    button.SetProperty(Actor::Property::ANCHOR_POINT, AnchorPoint::TOP_CENTER);
    // button.ClickedSignal().Connect(this, &ImageViewController::ToggleImageOnWindow);
    // button.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::WIDTH);
    // button.SetResizePolicy(ResizePolicy::USE_NATURAL_SIZE, Dimension::HEIGHT);
    button.SetResizePolicy(Dali::ResizePolicy::FILL_TO_PARENT, Dali::Dimension::ALL_DIMENSIONS);
    // std::string s = std::to_string(x);
    // button.SetProperty(Dali::Actor::Property::NAME, s);
    page.AddChild(button, Toolkit::TableView::CellPosition(0, 1));
  }

  GLuint LoadShader(GLenum type, const char* shaderSrc)
  {
    GLuint shader;
    GLint  compiled;

    //Create the shader object
    shader = glCreateShader(type);

    if(shader == 0)
    {
      return 0;
    }

    // Load the shader soure
    glShaderSource(shader, 1, &shaderSrc, NULL);

    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {
      GLint infoLen = 0;

      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

      if(infoLen > 1)
      {
        char* infoLog = (char*)malloc(sizeof(char) * infoLen);

        glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
        printf("LoadShader error %s\n", infoLog);

        free(infoLog);
      }

      glDeleteShader(shader);
      return 0;
    }

    return shader;
  }

  void initialize_gl()
  {
    fprintf(stderr, "%s\n", __FUNCTION__);
    char vShaderStr[] =
      "attribute vec4 vPosition;  \n"
      "void main()\n"
      "{\n"
      "     gl_Position = vPosition;\n"
      "}\n";

    char fShaderStr[] =
      "void main()\n"
      "{\n"
      "    gl_FragColor = vec4 (1.0, 0.0, 0.0, 1.0);\n"
      "}\n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint  linked;

    vertexShader   = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

    programObject = glCreateProgram();

    if(programObject == 0)
    {
      printf("programObject error\n");
      return;
    }

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    glLinkProgram(programObject);

    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if(!linked)
    {
      GLint infoLen = 0;
      glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

      if(infoLen > 1)
      {
        char* infoLog = (char*)malloc(sizeof(char) * infoLen);

        glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
        printf("error linking program:\n%s\n", infoLog);

        free(infoLog);
      }

      glDeleteProgram(programObject);
      return;
    }

    mProgramObject = programObject;

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  }

  int renderFrame_gl()
  {
    glClearColor(mClearColor, mClearColor, mClearColor, 1.0f);

    if(mClearColor < 0)
    {
      mClearColor = 1.0f;
    }
    else
    {
      mClearColor -= 0.01f;
    }
    GLfloat vVertices[] = {0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f};
    glViewport(0, 0, mGlViewWidth, mGlViewHeight);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mProgramObject);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    return 1;
  }

  void terminate_gl()
  {
    fprintf(stderr, "%s\n", __FUNCTION__);
  }

  void initialize_gl2()
  {
    fprintf(stderr, "%s\n", __FUNCTION__);
    char vShaderStr[] =
      "attribute vec4 vPosition;  \n"
      "void main()\n"
      "{\n"
      "     gl_Position = vPosition;\n"
      "}\n";

    char fShaderStr[] =
      "void main()\n"
      "{\n"
      "    gl_FragColor = vec4 (1.0, 0.0, 0.0, 1.0);\n"
      "}\n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint  linked;

    vertexShader   = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

    programObject = glCreateProgram();

    if(programObject == 0)
    {
      printf("programObject error\n");
      return;
    }

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    glLinkProgram(programObject);

    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if(!linked)
    {
      GLint infoLen = 0;
      glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

      if(infoLen > 1)
      {
        char* infoLog = (char*)malloc(sizeof(char) * infoLen);

        glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
        printf("error linking program:\n%s\n", infoLog);

        free(infoLog);
      }

      glDeleteProgram(programObject);
      return;
    }

    mProgramObject2 = programObject;

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  }

  int renderFrame_gl2()
  {
    glClearColor(mClearColor2, mClearColor2, mClearColor2, 0.3f);

    if(mClearColor2 < 0)
    {
      mClearColor2 = 1.0f;
    }
    else
    {
      mClearColor2 -= 0.01f;
    }
    GLfloat vVertices[] = {0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f};
    glViewport(0, 0, mGlViewWidth2, mGlViewHeight2);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mProgramObject2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    return 1;
  }

  void OnResize(int width, int height)
  {
    std::cout << "OnResize " << width << " " << height << std::endl;

    mGlViewWidth  = width;
    mGlViewHeight = height;
  }

  void OnResize2(int width, int height)
  {
    std::cout << "OnResize2 " << width << " " << height << std::endl;

    mGlViewWidth2  = width;
    mGlViewHeight2 = height;
  }

  bool OnTouch(Actor actor, const TouchEvent& touch)
  {
    if(touch.GetState(0) == PointState::UP)
    {
      mGlView.SetProperty(Actor::Property::SIZE, Vector2(720.0f, 1280.0f));
    }
    else
    {
      mGlView.SetProperty(Actor::Property::SIZE, Vector2(360.0f, 360.0f));
    }
    return true;
  }

  void OnKeyEvent(const KeyEvent& event)
  {
    if(event.GetState() == KeyEvent::DOWN)
    {
      if(IsKey(event, Dali::DALI_KEY_ESCAPE) || IsKey(event, Dali::DALI_KEY_BACK))
      {
        mApplication.Quit();
      }
    }
  }

private:
  Application& mApplication;
  Timer        mTimer;
  GlView       mGlView;
  GlView       mGlView2;
  int          mNumTouched;

  unsigned int mProgramObject;
  int          mGlViewWidth;
  int          mGlViewHeight;
  float        mClearColor;

  unsigned int mProgramObject2;
  int          mGlViewWidth2;
  int          mGlViewHeight2;
  float        mClearColor2;

  bool mGlViewInitialized;
};

int DALI_EXPORT_API main(int argc, char** argv)
{
  Application          application = Application::New(&argc, &argv);
  HelloWorldController test(application);
  application.MainLoop();
  return 0;
}
