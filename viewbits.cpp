//viewbits.cpp
//Copyright Matthew Chandler
//View file data as pixels

#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <GL/freeglut.h>

//file reading stops after this point (5MB)
#define MAX_SIZE 1024*1024*5

std::vector<unsigned char> data;
size_t data_size;

int win_width = 512, win_height = 512;
int zoom =1;
bool rgb = false;
int start_byte = 0;
GLuint texture =0;
bool key_down=false, key_up=false;
bool shift=false;
bool mouse_down=false, mouse_up=false;

//recalculate the texture with the new position/zoom
void recalc_image()
{
    if(rgb)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win_width/zoom,
            win_height/zoom, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[start_byte]);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, win_width/zoom,
            win_height/zoom, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &data[start_byte]);

    glutPostRedisplay();
}

//resize the display when window size is changed
void glutResize(int width = win_width, int height = win_height)
{
    width = glutGet(GLUT_WINDOW_WIDTH);
    //resize GL window
    if(height <= 2)
        height = 2;
    if(width <= 2)
        width = 2;
    win_width = width; win_height = height;
    glViewport(0,0,win_width,win_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,win_width,win_height,0);
    glMatrixMode(GL_MODELVIEW);

    //add black pixels so that we can have a fully black screen 
    //if we scroll to the end of the file w/o mem errors
    while(data.size() < data_size + win_width * win_height * 3)
        data.push_back(0);
    recalc_image();
}

//draw a quad with the texture showing file data
void glutDisplay()
{
    glBegin(GL_QUADS);
    glTexCoord2i(0,0); glVertex2i(0,0);
    glTexCoord2i(1,0); glVertex2i(win_width,0);
    glTexCoord2i(1,1); glVertex2i(win_width,win_height);
    glTexCoord2i(0,1); glVertex2i(0,win_height);
    glEnd();
    glutSwapBuffers();
}

//adjust scroll position
void glutIdle()
{
    int loopval = (shift)?4:1;
    for(int i=0; i<loopval; i++)
    {
        int add_val = win_width/zoom;
        if(rgb)
            add_val*=3;
        if(key_down || mouse_down)
        {
            if(start_byte + add_val < data_size)
                start_byte += add_val;
            recalc_image();
        }
        if(key_up || mouse_up)
        {
            if(start_byte - add_val>=0)
                start_byte -= add_val;
            recalc_image();
        }
    }
}

//modifier keys
//R toggles between grayscale and RGB
//Z zooms in, X zooms out
bool r_lock =false, z_lock=false, x_lock = false;
void glutKeyboard(unsigned char key, int x, int y)
{
    if(key == 27) //esc
        exit(0);
    else if(key == 'r')
    {
        rgb = !rgb;
        recalc_image();
    }
    else if(key == 'z')
    {
        if(zoom<512)
            zoom *=2;
        recalc_image();
    }
    else if(key == 'x')
    {
        if(zoom >= 2)
            zoom /= 2;
        recalc_image();
    }
}

//use arrow keys to scroll
void glutSpecial(int key, int x, int y)
{
    shift = (glutGetModifiers() == GLUT_ACTIVE_SHIFT);
    if(key == GLUT_KEY_DOWN)
    {
        key_down = true;
        glutIdle();
    }
    else if(key == GLUT_KEY_UP)
    {
        key_up = true;
        glutIdle();
    }
}
void glutSpecialUp(int key, int x, int y)
{
    shift = (glutGetModifiers() == GLUT_ACTIVE_SHIFT);
    if(key == GLUT_KEY_DOWN)
        key_down = false;
    else if(key == GLUT_KEY_UP)
        key_up = false;
}

//use scroll wheel
void glutMouse(int button, int state, int x, int y)
{
    if(button==3)
    {
        mouse_up = !(bool)(state);
        glutIdle();
    }
    else if(button==4)
    {
        mouse_down = !(bool)(state);
        glutIdle();
    }
}

int main(int argc, char** argv)
{
    //store file to mem
    int ctr =0;
    if(argc >1)
    {
        std::ifstream file(argv[1], std::ifstream::binary);
        if(!file.good())
        {
            std::cerr<<"Error reading file"<<std::endl;
            exit(1);
        }
        while(true)
        {
            unsigned char c = file.get();
            if(!file) break;
            data.push_back(c);
            if(++ctr>=MAX_SIZE)
            {
                std::cerr<<"file truncated at 5MB"<<std::endl;
                break;
            }
        }
        if(argc >3)
        {
            win_width = atoi(argv[2]);
            win_height = atoi(argv[3]);
        }
    }

#ifdef WIN32
    else //windows has issues reading binary from stdin
        exit(0);
#else
    else //read in file from stdin if no filename is passed if not windows
    {
        while(true)
        {
            unsigned char c = std::cin.get();
            if(!std::cin) break;
            data.push_back(c);
            if(++ctr>=MAX_SIZE)
            {
                std::cerr<<"file truncated at 5MB"<<std::endl;
                break;
            }
        }
    }
#endif
    std::cout<<"read "<<data.size()<<" bytes"<<std::endl;
    //save true file length
    data_size = data.size();

    //set up GLUT
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(win_width,win_height);
    glutCreateWindow("Memory map");
    glutIgnoreKeyRepeat(0);
    glutReshapeFunc(glutResize);
    glutDisplayFunc(glutDisplay);
    glutKeyboardFunc(glutKeyboard);
    glutSpecialFunc(glutSpecial);
    glutSpecialUpFunc(glutSpecialUp);
    glutMouseFunc(glutMouse);
    
    //set some openGL params
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glEnable(GL_TEXTURE_2D);

    //generate and set params for a texture
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //go
    glutMainLoop();
}
