#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <stdlib.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

struct Graphobj{
    float x,y;
    VAO * obj;
    int reflected;
    float startX;
    float startY;
    float slope;
    int status;
    double angle;
    double dangle;
    double newAngle;
    int brktype;
    float lengthx;
    float widthy;
    float diagonal;
    float speed;
};

typedef struct Graphobj Graphobj;
Graphobj greenbuck,redbuck;
Graphobj lasercanon;
Graphobj circle;
Graphobj laserTemplate;
Graphobj mirrors[3];
Graphobj bMirrors[3];
map <int, Graphobj> bricks;
map <int, Graphobj> laser;
double last_update_time2,current_time2;
double main_frame_current_time, main_frame_last_update;
float laserSpeed;
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
double mouse_x,mouse_y;
int winHieght,winWidth;
float brkrate;
int totBricks;
int totLasers;
float fallrate;
int onlyOnce;
int greenScore=0,redScore=0;
int totMirrors=3;
int togMouse=0;
float zoom=1;
float panx=0;
int toggleReflection=-1;
void addLaser();
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(yoffset==-1)
    {
        if(zoom < 1)
        {
            zoom += 0.01;
        }
    }
    else
    {
        if(zoom-0.01 > 0)
        {
            zoom -= (0.01);
        }
    }
}
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_T:
                togMouse=(togMouse+1)%2;
                if(togMouse==1)
                {
                    cout<<"Changed to keyboard controls"<<endl;
                }
                else
                {
                    cout<<"Changed to mouse controls"<<endl;
                }
                break;

            case GLFW_KEY_SPACE:
                addLaser();
                break;

            case GLFW_KEY_R:
                toggleReflection*=-1;
                if(toggleReflection==-1)
                {
                    cout<<"Changed to reflection"<<endl;
                }
                else
                {
                    cout<<"Changed to refraction"<<endl;
                }
                break;

            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}
void addLaser()
{
     current_time2=glfwGetTime();
     if(current_time2 - last_update_time2 >= 1)
     {
        /*******************************Make the effing laser******************************/
        //cout<<"Laser Angle: "<<lasercanon.angle<<endl;
        laser[totLasers].angle=lasercanon.angle;
        laser[totLasers].x=-3.9999;
        laser[totLasers].startX=-4;
        laser[totLasers].startY=lasercanon.y;
        laser[totLasers].slope=tan(laser[totLasers].angle);
        laser[totLasers].y=laser[totLasers].slope*(laser[totLasers].x+4)+laser[totLasers].startY;
        laser[totLasers].status=1;
        laser[totLasers].reflected=-1;
        laser[totLasers].speed=laserSpeed;
        totLasers++;
        last_update_time2=current_time2;
        //cout<<"Total Lasers: "<<totLasers<<endl;
        /**********************************************************************************/
    }
    else
    {
        cout<<"Recharging, Wait!!!"<<endl;
    }
}
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if(onlyOnce%2==0 && togMouse==0)
            {
                addLaser();
            }
            onlyOnce=(onlyOnce+1)%2;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= 1;
            }
            break;
        default:
            break;
    }
}
/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f/zoom, 4.0f/zoom, -4.0f/zoom, 4.0f/zoom, 0.1f, 500.0f);
}

int checkCollisionLB(Graphobj l, Graphobj b)
{
    float x1 = l.x+ laserTemplate.lengthx/2;
    float y1 = l.y+ laserTemplate.widthy/2;
    float x2 = b.x+ b.lengthx/2;
    float y2 = b.y+ b.widthy/2;
    //cout<<fabs(x2-x1)<<'\t'<<fabs(y2-y1)<<endl;
    //if(abs(x2 - x1) <= ((laserTemplate.lengthx+b.lengthx)/2) && abs(y2-y1)<=(laserTemplate.diagonal+b.widthy)/2)
    if(fabs(x2 - x1) <= 0.1 && fabs(y2-y1)<=0.2)
    {
        //cout<<"Collision!!!"<<endl;
        return 1;
    }
    else
    {
        //cout<<"No Collision"<<endl;
        return 0;
    }
}

int checkCollisionLM(Graphobj* l,int M)
{
    float x = l->x+laserTemplate.lengthx;
    float y = l->y+laserTemplate.widthy;
    float m =  mirrors[M].slope;
    float x1 = mirrors[M].x;
    float y1 = mirrors[M].y;
    //if(M==2)
    //    cout<<x<<" "<<x1-cos(1.57079- mirrors[M].angle)*0.7<<" "<<x1+cos(1.57079- mirrors[M].angle)*0.7<<" "<<M<<" Distance: "<<fabs(m*x-y+y1-m*x1)/sqrt(m*m+1)<<endl;
    //cout<<l.reflected<<endl;
    if((x<=x1+cos(1.57079-mirrors[M].angle)*0.7 && x>=x1-cos(1.57079- mirrors[M].angle)*0.7) || ( y<=sin(1.57079-mirrors[M].angle)*0.7+y1 && y>=y1-sin(1.57079-mirrors[M].angle)*0.7 ) )
    {
        float distance=(fabs(m*x-y+y1-m*x1)/sqrt(m*m+1));
        if(distance<=0.05)
        {
            if(l->reflected!=M)
            {
                l->reflected=M;
                l->startX=l->x;
                l->startY=l->y;
                //cout<<mirrors[M].angle<<' '<<l->angle<<endl;
                l->angle=(2*mirrors[M].angle - l->angle);
                l->speed*=toggleReflection;
                //cout<<"Angle: "<<l->angle<<endl;
                l->slope=tan(l->angle);
                //cout<<"Reflect mfer  "<<distance<<"M: "<<M<<"l->m"<<l->reflected<<endl;

            }
        }
    }

    return 1;
}
VAO * mirrorTemplate;
void createMirror()
{
    static const GLfloat vertex_buffer_data [] = {
      0,0,0, // vertex 0
      0,0.7,0, // vertex 1
      0,-0.7,0, // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
      1,0,0, // color 0
      0,1,0, // color 1
      0,0,1, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    mirrorTemplate = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void createLaser()
{
    static const GLfloat vertex_buffer_data [] = {
      0,0,0, // vertex 0
      0.2,0.05,0, // vertex 1
      0.2,0,0, // vertex 2
      0,0.05,0,
      0.2,0.05,0,
      0,0,0
    };

    static const GLfloat color_buffer_data [] = {
      1,0,0, // color 0
      0,0,1, // color 1
      0,1,0, // color 2
      0,1,0, // color 0
      0,0,1, // color 1
      1,0,0, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    laserTemplate.obj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    laserTemplate.lengthx=0.2;
    laserTemplate.widthy=0.05;
    laserTemplate.diagonal=sqrt((0.2*0.2)+(0.05*0.05));
}

void createCircle(float cenx,float ceny, float angle,float radius)
{
    float dtht=10;
    GLfloat vertex_buffer_data[int((angle/dtht)*9)+1];
    GLfloat color_buffer_data[int((angle/dtht)*9)+1];
    float pi=3.141596f;
    for(int i=0;i<int(angle/dtht)*9+1;i++)
    {
        color_buffer_data[i]=0;
    }
    for(int i=0;i<int(angle/dtht);i++)
    {
        color_buffer_data[9*i]=1;
        color_buffer_data[9*i+4]=1;
        color_buffer_data[9*i+8]=1;
        vertex_buffer_data[9*i]=radius*cos((i+1)*dtht*pi/180);
        vertex_buffer_data[9*i+1]=radius*sin((i+1)*dtht*pi/180);
        vertex_buffer_data[9*i+2]=0;
        vertex_buffer_data[9*i+3]=radius*cos(i*dtht*pi/180);
        vertex_buffer_data[9*i+4]=radius*sin(i*dtht*pi/180);
        vertex_buffer_data[9*i+5]=0;
        vertex_buffer_data[9*i+6]=0;
        vertex_buffer_data[9*i+7]=0;
        vertex_buffer_data[9*i+8]=0;
        /*for(int j=0;j<9;j++)
        {
            cout<<vertex_buffer_data[9*i+j]<<',';
            if(j%3==0)
                cout<<endl;
        }*/
    }
    circle.obj = create3DObject(GL_TRIANGLES, 3*angle/dtht, vertex_buffer_data, color_buffer_data, GL_FILL);
}
// Creates the triangle object used in this sample code
/*VAO * point;
void creatPoint(float x,float y)
{
    x+=0.05;
    y+=0.1;
    static const GLfloat vertex_buffer_data [] = {
      x,y,0, // vertex 0
      x+0.05,y,0, // vertex 1
      x,y+0.05,0, // vertex 2
    };
    GLfloat color_buffer_data [] = {
      0,0,0, // color 0
      0,0,0, // color 1
      0,0,0, // color 2 // color 2
    };
    point= create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}*/
void createCanon ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0,0.25f,0, // vertex 0
    0,-0.25,0, // vertex 1
    1,0.04,0, // vertex 2
    1,0.04,0,
    0,-0.25,0,
    1,-0.04,0
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,0,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 0
    0,0,0, // color 1
    1,0,0, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  lasercanon.obj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  lasercanon.angle=0;
  lasercanon.y=0;
}
VAO* behindMirror;
void createBehindMirror()
{
    static const GLfloat vertex_buffer_data [] = {
      0,0.7,0, // vertex 0
      0,-0.7,0, // vertex 1
      0.05,0.7,0, // vertex 2
      0.05,0.7,0,
      0,-0.7,0,
      0.05,-0.7,0
    };

    static const GLfloat color_buffer_data [] = {
        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 0
        0,0,0, // color 1
        0,0,0, // c
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    behindMirror = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
VAO* createBucket(int color)
{
    static const GLfloat vertex_buffer_data [] = {
      -0.3,-3.6,0, // vertex 0
      0.3,-3.6,0, // vertex 1
      0.4,-4,0, // vertex 2
      -0.3,-3.6,0,
      0.4,-4,0,
      -0.4,-4,0
    };
    GLfloat x=0,y=0;
    if(color==1)
        y=1;
    if(color==2)
        x=1;
    GLfloat color_buffer_data [] = {
      x,y,0, // color 0
      x,y,0, // color 1
      x,y,0, // color 2
      x,y,0, // color 0
      x,y,0, // color 1
      x,y,0, // color 2
    };
    return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void addBrick(int brkno,float brkpos)
{
    int a,b;
    static const GLfloat vertex_buffer_data [] = {
      0,0,0, // vertex 0
      0.2,0,0, // vertex 1
      0.2,0.35,0, // vertex 2
      0.2,0.35,0,
      0,0,0,
      0,0.35,0
    };
    GLfloat color_buffer_data [] = {
      0,0,0, // color 0
      0,0,0, // color 1
      0,0,0, // color 2
      0,0,0, // color 0
      0,0,0, // color 1
      0,0,0, // color 2
    };
    for(int i=0;i<18;i++)
    {
        if(brkno==1)
        {
            if(i%3==1)
            {
                color_buffer_data[i]=1;
                //cout<<"Color 1 in brick "<<totBricks;
            }
        }
        if(brkno==2)
        {
            if(i%3==0)
            {
                color_buffer_data[i]=1;
            }
        }
    }
    // create3DObject creates and returns a handle to a VAO that can be used later
     bricks[totBricks].obj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
     bricks[totBricks].y=3.8;
     bricks[totBricks].x=brkpos;
     bricks[totBricks].lengthx=0.2;
     bricks[totBricks].widthy=0.35;
     bricks[totBricks].brktype=brkno;
     bricks[totBricks].status=1;
     totBricks++;
}

double zoom_pan_speed = 0.1;
double cannon_speed = 0.1;
double angle_speed = 0.1;
double brick_speed = 0.1;
double buck_speed = 0.1;


void draw (GLFWwindow* window)
{
    double dt = main_frame_current_time - main_frame_last_update;
    cout<<dt<<" "<<main_frame_current_time<<" "<<main_frame_last_update<<endl;
      // clear the color and depth in the frame buffer
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // use the loaded shader program
      // Don't change unless you know what you are doing
      glUseProgram (programID);

      // Eye - Location of camera. Don't change unless you are sure!!
      glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
      // Target - Where is the camera looking at.  Don't change unless you are sure!!
      glm::vec3 target (0, 0, 0);
      // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
      glm::vec3 up (0, 1, 0);

      // Compute Camera matrix (view)
      // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
      //  Don't change unless you are sure!!
      Matrices.view = glm::lookAt(glm::vec3(panx,0,3), glm::vec3(panx,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
      Matrices.projection = glm::ortho(-4.0f*zoom, 4.0f*zoom, -4.0f*zoom, 4.0f*zoom, 0.1f, 500.0f);
      // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
      //  Don't change unless you are sure!!
      glm::mat4 VP = Matrices.projection * Matrices.view;

      // Send our transformation to the currently bound shader, in the "MVP" uniform
      // For each model you render, since the MVP will be different (at least the M part)
      //  Don't change unless you are sure!!
      glm::mat4 MVP;	// MVP = Projection * View * Model
    /***Handling keyboard***********************************************/
      if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS)
      {
          if(lasercanon.y < 4)
          {
              lasercanon.y += cannon_speed*dt;
          }
      }
      else if(glfwGetKey(window,GLFW_KEY_F)==GLFW_PRESS)
      {
          if(lasercanon.y > -4)
          {
              lasercanon.y -= cannon_speed*dt;
          }
      }
      if(glfwGetKey(window,GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS)
      {
          if(redbuck.x < 4.9)
          {
              redbuck.x += buck_speed*dt;
          }
      }
      else if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS)
      {
          if(redbuck.x> -4)
          {
              redbuck.x -= buck_speed*dt;
          }
      }
      if(glfwGetKey(window,GLFW_KEY_RIGHT_ALT)==GLFW_PRESS)
      {
          if(greenbuck.x < 4.9)
          {
              greenbuck.x += buck_speed*dt;
          }
      }
      else if(glfwGetKey(window,GLFW_KEY_LEFT_ALT)==GLFW_PRESS)
      {
          if(greenbuck.x > -4)
          {
              greenbuck.x -= buck_speed*dt;
          }
      }
      if(glfwGetKey(window,GLFW_KEY_M)==GLFW_PRESS)
      {
          if(brkrate < 0.1)
          {
              brkrate += (0.001)*dt;
          }
      }
      else if(glfwGetKey(window,GLFW_KEY_N)==GLFW_PRESS)
      {
          if(brkrate-0.02 > 0.005)
          {
              brkrate -= (0.001)*dt;
          }
      }
      if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS && togMouse==1)
      {
          if(lasercanon.newAngle < 1.57079)
          {
              lasercanon.newAngle += angle_speed*dt;
          }
      }
      else if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS && togMouse==1)
      {
          if(lasercanon.newAngle > -1.57079)
          {
              lasercanon.newAngle -= angle_speed*dt;
          }
      }
      if(glfwGetKey(window,GLFW_KEY_DOWN)==GLFW_PRESS)
      {
          if(zoom < 1)
          {
              zoom += zoom_pan_speed*dt;
          }
      }
      else if(glfwGetKey(window,GLFW_KEY_UP)==GLFW_PRESS)
      {
          if(zoom-0.01 > 0)
          {
              zoom -= zoom_pan_speed*dt;
          }
      }
      if(glfwGetKey(window,GLFW_KEY_RIGHT)==GLFW_PRESS)
      {
          if(panx < 4)
          {
              panx += zoom_pan_speed*dt;
          }
      }
      else if(glfwGetKey(window,GLFW_KEY_LEFT)==GLFW_PRESS)
      {
          if(panx-0.01 > -4)
          {
              panx -= zoom_pan_speed*dt;
          }
      }
      if(glfwGetKey(window,GLFW_KEY_V)==GLFW_PRESS)
      {
          if(laserSpeed < 15)
          {
              laserSpeed += 0.005;
          }
      }
      else if(glfwGetKey(window,GLFW_KEY_C)==GLFW_PRESS)
      {
          if(laserSpeed > 1)
          {
              laserSpeed -= (0.005);
          }
      }
      glfwGetWindowSize(window,&winWidth, &winHieght);
      glfwGetCursorPos(window, &mouse_x, &mouse_y);
      if(togMouse==1)
      {
          if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS)
          {
              float X = (mouse_x-winWidth/2)/100;
              float Y = -(mouse_y-winWidth/2)/100;
              //cout<<"Mouse Pressed"<<(mouse_x-winWidth/2)/100<<" "<<-(mouse_y+winWidth/2)/100<<endl;
              if(X>=-4-0.8 && X<=-4+0.8 && Y>=lasercanon.y-0.8 && Y<=lasercanon.y+0.8)
              {
                  //cout<<"Mouse Pressed"<<(mouse_x-winWidth/2)/100<<" "<<(mouse_y-winWidth/2)/100<<endl;
                  lasercanon.y=-(mouse_y-winHieght/2)/100;
              }
              else if(X>=redbuck.x-0.4 && X<=redbuck.x+0.4 && Y<=-3.6 && Y>=-4)
              {
                  redbuck.x=X;
              }
              else if(X>=greenbuck.x-0.4 && X<=greenbuck.x+0.4 && Y<=-3.6 && Y>=-4)
              {
                  greenbuck.x=X;
              }
          }
      }
      /***************************************************Mirrors********************************************************/
      for(int ij=0;ij<totMirrors;ij++)
      {
          Matrices.model = glm::mat4(1.0f);
          glm::mat4 translatemirror = glm::translate (glm::vec3(mirrors[ij].x,mirrors[ij].y, 0));        // glTranslatef
          glm::mat4 rotatemirror = glm::rotate((float)mirrors[ij].angle, glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
          Matrices.model *= (translatemirror*rotatemirror);
          MVP = VP * Matrices.model;
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          draw3DObject(mirrorTemplate);
      }
      if(toggleReflection==-1)
      {
          for(int ij=0;ij<totMirrors;ij++)
          {
              Matrices.model = glm::mat4(1.0f);
              glm::mat4 translatemirror = glm::translate (glm::vec3(mirrors[ij].x,mirrors[ij].y, 0));        // glTranslatef
              glm::mat4 rotatemirror = glm::rotate((float)mirrors[ij].angle, glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
              Matrices.model *= (translatemirror*rotatemirror);
              MVP = VP * Matrices.model;
              glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
              draw3DObject(behindMirror);
          }
      }
      /*************************************************Laser loop******************************************************************/
      for(map<int,Graphobj>::iterator it=laser.begin();it!=laser.end();)
      {
          for(map<int,Graphobj>::iterator it2=bricks.begin();it2!=bricks.end();it2++)
          {
              //cout<<"In bricks"<<endl;
              //if(it2->second.brktype==0)
            //  {
                  int forst = checkCollisionLB(it->second,it2->second);
                  forst=(forst+1)%2;
                  it2->second.status=forst;
                  //it->second.status=forst;
                  if(forst==0)
                  {
                      //cout<<"Breaking";
                      break;
                  }
              //}
          }
          for(int i =0;i<totMirrors;i++)
          {
              checkCollisionLM(&(it->second),i);
          }
          Matrices.model = glm::mat4(1.0f);
          if(it->second.x>-4 && it->second.x<4 && it->second.y<4 && it->second.y>-4 && it->second.status==1)
          {
              Matrices.model = glm::mat4(1.0f);
              glm::mat4 translateLaser = glm::translate (glm::vec3(it->second.x,it->second.y, 0));
              it->second.x+=it->second.speed*dt*cos(it->second.angle);        // glTranslatef
              it->second.y=it->second.slope*(it->second.x-it->second.startX)+it->second.startY;
              glm::mat4 rotateLaser = glm::rotate((float)(it->second.angle), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
              Matrices.model *= (translateLaser * rotateLaser);
              MVP = VP * Matrices.model;
              glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
              draw3DObject(laserTemplate.obj);
              //draw3DObject(point);
              ++it;
          }
          else
          {
              laser.erase(it++);
          }
      }

      /****************************************** Buckets ************************************************************************/
      /*************************Green**********************/
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 translategbuck = glm::translate (glm::vec3(greenbuck.x,greenbuck.y, 0));        // glTranslatef
      Matrices.model *= (translategbuck);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(greenbuck.obj);
      /*************************Red***********************/
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 translaterbuck = glm::translate (glm::vec3(redbuck.x,redbuck.y, 0));        // glTranslatef
      Matrices.model *= (translaterbuck);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(redbuck.obj);
      /****************************             Laser Canon                                     ***********************************/
      Matrices.model = glm::mat4(1.0f);

     // cout<<"x: "<<mouse_x-400<<"   y: "<<mouse_y-400<<endl;
      if(togMouse==0)
        lasercanon.newAngle=-(atan((mouse_y- double(winHieght/2)+double(lasercanon.y)*100)/mouse_x));//+atan(double(lasercanon.y)/4));
      //cout<<lasercanon.newAngle<<' '<<lasercanon.y<<endl;
      lasercanon.angle=lasercanon.newAngle;
      glm::mat4 translateTriangle = glm::translate (glm::vec3(-4.0f, 0.0f+lasercanon.y, 0.0f)); // glTranslatef
      glm::mat4 rotateTriangle = glm::rotate(float(lasercanon.newAngle), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
      glm::mat4 triangleTransform = translateTriangle*rotateTriangle;
      Matrices.model *= triangleTransform;
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(lasercanon.obj);
     /***************************************************************************************************************/
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 translateRectangle = glm::translate (glm::vec3(-4, 0+lasercanon.y, 0));        // glTranslatef
      glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      Matrices.model *= (translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(circle.obj);
     /**************************************************************************************************************/
     /*Matrices.model = glm::mat4(1.0f);
     glm::mat4 tp = glm::translate (glm::vec3(-1.9,1.5, 0));        // glTranslatef
     Matrices.model *= (tp);
     MVP = VP * Matrices.model;
     glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
     draw3DObject(point);
     Matrices.model = glm::mat4(1.0f);
     tp = glm::translate (glm::vec3(-1.08,1.5, 0));        // glTranslatef
     Matrices.model *= (tp);
     MVP = VP * Matrices.model;
     glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
     draw3DObject(point);*/
    /**********************************************   Handling Bricks   ****************************************************************/
    //cout<<"YOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"<<endl;
    for(map<int,Graphobj>::iterator it=bricks.begin();it!=bricks.end();)
    {
        Matrices.model = glm::mat4(1.0f);
        if(it->second.y+0.3 + it->second.widthy < -3 && it->second.x >= (greenbuck.x-0.3) && it->second.x <= (greenbuck.x+0.3-it->second.lengthx))
        {
            if(it->second.brktype==1)
            {
                it->second.status=0;
                cout<<"Green in the bag. Score: "<<++greenScore<<endl;
            }
            else if(it->second.brktype==0)
            {
                cout<<"Opps,caught the black one! Game Over!"<<endl;
                cout<<"Green score: "<<greenScore<<endl;
                cout<<"Red score: "<<redScore<<endl;
                cout<<"Total score: "<<greenScore+redScore<<endl;
                quit(window);
            }
        }
        if(it->second.y+0.3+it->second.widthy<-3 && it->second.x>=(redbuck.x-0.3) && it->second.x<=(redbuck.x+0.3-it->second.lengthx))
        {
            if(it->second.brktype==2)
            {
                it->second.status=0;
                cout<<"Red in the bag. Score: "<<++redScore<<endl;
            }
            else if(it->second.brktype==0)
            {
                cout<<"Opps,caught the black one! Game Over!"<<endl;
                cout<<"Green score: "<<greenScore<<endl;
                cout<<"Red score: "<<redScore<<endl;
                cout<<"Total score: "<<greenScore+redScore<<endl;
                quit(window);
            }
        }
        // if(it->second.brktype==0 && ((it->second.y+0.3+it->second.widthy<-3 && it->second.x>=(greenbuck.x-0.3) && it->second.x<=(greenbuck.x+0.3-it->second.lengthx))||(it->second.y+0.3+it->second.widthy<-3 && it->second.x>=(redbuck.x-0.3) && it->second.x<=(redbuck.x+0.3-it->second.lengthx))))
        // {
        //     cout<<"Sorry! Black one caught!!!"<<endl;
        //     quit(window);
        // }
        if(it->second.y>-4.5 && it->second.status==1)
        {
            glm::mat4 translatebrk = glm::translate (glm::vec3(it->second.x,it->second.y, 0));        // glTranslatef
            it->second.y= (it->second.y)-brkrate*dt;
            //cout<<it->second.y<<" "<<it->second.x<<endl;
            //rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
            Matrices.model *= (translatebrk);
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(it->second.obj);
            //draw3DObject(point);
            ++it;
        }
        else
        {
            bricks.erase(it++);
        }
    }
/******************************************Try***********************************************************************/
/*    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(0,0, 0));        // glTranslatef
    rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(laserTemplate.obj);
*/

/**********************************************************************************************************************/
  // Increment angles
  float increments = 0.1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*dt*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + laserSpeed*dt*10*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createCanon (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();
    createCircle(0,0,360,0.8);
    createLaser();
    createMirror();
    createBehindMirror();
    greenbuck.obj = createBucket(1);
    redbuck.obj = createBucket(2);
    greenbuck.x=-3;
    redbuck.x=3;
    mirrors[0].x=1;
    mirrors[0].y=-2;
    mirrors[0].angle=-0.7;
    mirrors[0].slope=tan(1.57079+mirrors[0].angle);
    mirrors[1].x=1;
    mirrors[1].y=2;
    mirrors[1].angle=0.7;
    mirrors[1].slope=tan(1.57079+mirrors[1].angle);
    mirrors[2].x=-1.5;
    mirrors[2].y=0;
    mirrors[2].angle=0;
    mirrors[2].slope=tan(1.57079+mirrors[2].angle);
    //creatPoint(0,0);
    //addBrick(1,2);
    // Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0,34, 51, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
    glfwSetScrollCallback(window, scroll_callback);
    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 800;
	int height = 800;
    srand(time(NULL));
    GLFWwindow* window = initGLFW(width, height);
    totBricks=0;
    totLasers=0;
    laserSpeed=15;
    brkrate=0.1;
    fallrate=2;
    onlyOnce=0;
	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    last_update_time2 = glfwGetTime();
    main_frame_last_update = glfwGetTime();
    int brkno;
    float brkpos;
    main_frame_current_time = glfwGetTime();
    main_frame_last_update = main_frame_current_time;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
        main_frame_current_time = glfwGetTime();
        // OpenGL Draw commands
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= fallrate) { // atleast 2s elapsed since last frame
            // do something every 2 seconds ..
            for(int i=0;i<=int(rand()%4);i++)
            {
                brkno=rand()%3;
                brkpos=((rand()%800)-300.0)/100;
                addBrick(brkno,brkpos);
            }
            last_update_time = current_time;
        }
        main_frame_last_update = main_frame_current_time;

    }

    glfwTerminate();

//    exit(EXIT_SUCCESS);
}
