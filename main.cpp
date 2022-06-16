#include "src/Graphics/window.hpp"
#include "src/Graphics/shader.hpp"
#include "src/Utils/camera.hpp"

#include "src/Buffers/buffer.hpp"
#include "src/Buffers/indexbuffer.hpp"
#include "src/Buffers/vertexarray.hpp"

#include "src/Graphics/renderer.hpp"
#include "src/Graphics/simple2drenderer.hpp"

#define using_buffers 1
using namespace Graphics;

int main()
{
    //creating the window
    Window window("title", 960, 540);
    glClearColor(0.2, 0.3, 0.8, 1.0);

    //making a ground plain for the rectangle to bounce off of
    b2Vec2 gravity(0.0f, -0.1f);
    b2World world(gravity);
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -20.0f);
    b2Body* groundBody = world.CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 19.0f);
    groundBody->CreateFixture(&groundBox, 0.0f);

    //making the physics based object for the bouncing rectangle
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0f, 10.0f);
    b2Body* body = world.CreateBody(&bodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(1.0f, 1.0f);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body->CreateFixture(&fixtureDef);

    //variables for updating all physics in game
    float timeStep = 1.0f/2.1f;
    int32 velocityIterations = 6;
    int32 positionIterations = 2;

    //creation of shaders and projection matrix
    Shader shader("../resources/Shaders/VertShader", "../resources/Shaders/FragShader");
    shader.bind();
	Camera ortho = Camera::Orthographic(0, 16, 0, 9, -1, 1);
	shader.setUniformMat4("pr_matrix", ortho);
	shader.setUniformMat4("ml_matrix", Camera::translation(glm::vec3(4, 3, 0)));

    //sprite generators
	Renderer sprite(glm::vec3(3, 5, 0), glm::vec2(1, 1), glm::vec4(1, 0, 1, 1), shader);
	Renderer sprite2(glm::vec3(7, 1, 0), glm::vec2(2, 3), glm::vec4(0.2f, 0, 1, 1), shader);
	simple2drenderer renderer;

    //this is to tell the computer to draw a bright circle that fades to black
	shader.setUniform2f("light_pos", glm::vec2(4.0f, 1.5f));
	shader.setUniform4f("colour", glm::vec4(0.2f, 0.3f, 0.8f, 1.0f));

    //values for imgui
	bool show_demo_window = true;

    static float f = 0.0f;
    static int counter = 0;

    //values for the fixed update
    static double limitFPS = 1.0 / 60.0;
    double lastTime = glfwGetTime(), timer = lastTime;
    double deltaTime = 0, nowTime = 0;
    int frames = 0 , updates = 0;

    while (!window.Closed())
    {
        window.Clear();
        
        //stuff to calculate delta time for the fixed update
        nowTime = glfwGetTime();
        deltaTime += (nowTime - lastTime) / limitFPS;
        lastTime = nowTime;

        //begining the on screen gui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        //checks is the key TAB was pressed if so freeze the moving object
        if (window.isKeyPressed(GLFW_KEY_TAB)){
            body->SetEnabled(0);
        } else {
            body->SetEnabled(1);
        }

        //for mouse positioning
        double x, y;
        window.getmouseposition(x, y);
        
        //foe the bright light
        shader.setUniform2f("light_pos", glm::vec2(3, 5.5));

        //ImGui box which makes the inner window
        ImGui::Begin("FPS Checker.");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("mouse X:%f, mouse Y:%f", (float)(x * 16.0f / 960.0f), (float)(9.0f - y * 9.0f / 540.0f));
        ImGui::Text("pressing tab stops the bouncing rectangle.");
        ImGui::Text("state of key tab:%d. 1 means it is pressed \nand 0 means it has not been pressed", window.isKeyPressed(GLFW_KEY_TAB));
        ImGui::End();

        //this sends the sprites to get rendered
		renderer.submit(&sprite);
		renderer.submit(&sprite2);
		renderer.flush();

        //fixed update
        while (deltaTime >= 1.0){
            //this is for the moving rectangle with physics
            world.Step(timeStep, velocityIterations, positionIterations);
            b2Vec2 position = body->GetPosition();
            glm::float32 angle = body->GetAngle();
            if (body->GetLinearVelocity().y == 0)
                body->ApplyLinearImpulseToCenter(b2Vec2(0, 5.0f), true);
            sprite2.Position(glm::vec3(1, position.y, 0));
            
            updates++;
            deltaTime--;
        }
        //rendering to the screen
		ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.Update();

        //debug stuff for fixed update
        frames++;
        if (glfwGetTime() - timer > 1.0) {
            timer ++;
            std::cout << "FPS: " << frames << " Updates:" << updates << std::endl;
            updates = 0, frames = 0;
        }    
    }
    //when the application closes it deletes all instances of its self to free up computer resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
