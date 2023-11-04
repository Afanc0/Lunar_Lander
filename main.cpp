#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define NUMBER_OF_BLOCKS 24

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

#include <thread>
#include <chrono>

// ————— STRUCTS AND ENUMS —————//
struct GameState
{
    Entity* player;
    Entity* stone_block[NUMBER_OF_BLOCKS];
    Entity* text;
    Entity* land_indictor;
    Entity* rocket_fire;
    Entity* fuel;
};

// ————— CONSTANTS ————— //
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1f,
BG_BLUE = 0.1f,
BG_GREEN = 0.1f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[] = "assets/space_ship.png",
STONEBLOCK_FILEPATH[] = "assets/StoneFloorTexture_1.png",
GAMEOVER_FILEPATH[] = "assets/Failed.png",
VICTORY_FILEPATH[] = "assets/Passed.png",
LAND_FILEPATH[] = "assets/land_indicator.png",
BOOST_FILEPATH[] = "assets/fire1.png",
FUEL_FILEPATH[] = "assets/fuel.png";


const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;

float gravity = -0.2f;

int fuel_tank = 5000;
int fuel_usage = 1;

// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Lunar Rover!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ————— PLAYER ————— //
    g_game_state.player = new Entity();
    g_game_state.player->set_position(glm::vec3(-3.75f, 3.0f, 0.0f));
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, gravity, 0.0f));
    g_game_state.player->set_texture_id(load_texture(SPRITESHEET_FILEPATH));

    // ————— STONE BLOCK (TERRAIN) ————— // 

    GLuint stone_block_texture_id = load_texture(STONEBLOCK_FILEPATH);
    for (int i = 0; i < NUMBER_OF_BLOCKS - 4; i++)
    {
        g_game_state.stone_block[i] = new Entity();
        g_game_state.stone_block[i]->set_texture_id(stone_block_texture_id);
    }

    float next_block_position_x = -4.5;
    for (int i = 0; i < NUMBER_OF_BLOCKS - 4; i++) {
        g_game_state.stone_block[i]->set_position(glm::vec3(next_block_position_x++, -3.25f, 0.0f));
        g_game_state.stone_block[i]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    }

    g_game_state.stone_block[10] = new Entity();
    g_game_state.stone_block[10]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[10]->set_position(glm::vec3(-2.5f, -2.25f, 0.0f));
    g_game_state.stone_block[10]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[11] = new Entity();
    g_game_state.stone_block[11]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[11]->set_position(glm::vec3(-2.5f, -2.25f, 0.0f));
    g_game_state.stone_block[11]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[12] = new Entity();
    g_game_state.stone_block[12]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[12]->set_position(glm::vec3(-2.5f, -1.25f, 0.0f));
    g_game_state.stone_block[12]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[13] = new Entity();
    g_game_state.stone_block[13]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[13]->set_position(glm::vec3(-1.5f, -2.25f, 0.0f));
    g_game_state.stone_block[13]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[14] = new Entity();
    g_game_state.stone_block[14]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[14]->set_position(glm::vec3(1.5f, -2.25f, 0.0f));
    g_game_state.stone_block[14]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[15] = new Entity();
    g_game_state.stone_block[15]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[15]->set_position(glm::vec3(4.5f, -2.25f, 0.0f));
    g_game_state.stone_block[15]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[15] = new Entity();
    g_game_state.stone_block[15]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[15]->set_position(glm::vec3(0.5f, -2.25f, 0.0f));
    g_game_state.stone_block[15]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
     
    g_game_state.stone_block[16] = new Entity();
    g_game_state.stone_block[16]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[16]->set_position(glm::vec3(-1.5f, -1.25f, 0.0f));
    g_game_state.stone_block[16]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[17] = new Entity();
    g_game_state.stone_block[17]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[17]->set_position(glm::vec3(-1.5f, -0.25f, 0.0f));
    g_game_state.stone_block[17]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[18] = new Entity();
    g_game_state.stone_block[18]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[18]->set_position(glm::vec3(0.5f, -1.25f, 0.0f));
    g_game_state.stone_block[18]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[19] = new Entity();
    g_game_state.stone_block[19]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[19]->set_position(glm::vec3(-0.5f, -2.25f, 0.0f));
    g_game_state.stone_block[19]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[20] = new Entity();
    g_game_state.stone_block[20]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[20]->set_position(glm::vec3(4.5f, -2.25f, 0.0f));
    g_game_state.stone_block[20]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[21] = new Entity();
    g_game_state.stone_block[21]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[21]->set_position(glm::vec3(-4.5f, -2.25f, 0.0f));
    g_game_state.stone_block[21]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[22] = new Entity();
    g_game_state.stone_block[22]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[22]->set_position(glm::vec3(-3.5f, -2.25f, 0.0f));
    g_game_state.stone_block[22]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    g_game_state.stone_block[23] = new Entity();
    g_game_state.stone_block[23]->set_texture_id(stone_block_texture_id);
    g_game_state.stone_block[23]->set_position(glm::vec3(-4.5f, -1.25f, 0.0f));
    g_game_state.stone_block[23]->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    // ————— LAND INDICATOR ————— //

    g_game_state.land_indictor = new Entity();
    g_game_state.land_indictor->set_texture_id(load_texture(LAND_FILEPATH));
    g_game_state.land_indictor->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    g_game_state.land_indictor->set_position(glm::vec3(3.5f, -2.25f, 0.0f));

    // ————— TEXT ————— //

    g_game_state.text = new Entity();
    g_game_state.text->set_texture_id(load_texture(GAMEOVER_FILEPATH));
    g_game_state.text->set_position(glm::vec3(0.0f, 1.0f, 0.0f));

    // ————— ROCKET FIRE ————— //

    g_game_state.rocket_fire = new Entity();
    g_game_state.rocket_fire->set_texture_id(load_texture(BOOST_FILEPATH));
    g_game_state.rocket_fire->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    g_game_state.rocket_fire->set_position(glm::vec3(0.0f, 1.0f, 0.0f));
    g_game_state.rocket_fire->set_rotate(0.0f);

    // ————— FUEL BAR ————— //

    g_game_state.fuel = new Entity();
    g_game_state.fuel->set_texture_id(load_texture(FUEL_FILEPATH));
    g_game_state.fuel->set_scale(glm::vec3(1.5f, 0.25f, 0.0f));
    g_game_state.fuel->set_position(glm::vec3(4.0f, 3.5f, 0.0f));

    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q: g_game_is_running = false;
            default:     break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT] && fuel_tank != 0)
    {
        g_game_state.player->move_left();
        g_game_state.player->set_rotate(1.6f);


        g_game_state.rocket_fire->set_position(glm::vec3(
            g_game_state.player->get_position().x + 0.5f,
            g_game_state.player->get_position().y,
            g_game_state.player->get_position().z
        ));

        g_game_state.rocket_fire->set_rotate(1.6f);
        g_game_state.rocket_fire->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

        fuel_tank -= fuel_usage;
        g_game_state.fuel->set_scale(glm::vec3(
            g_game_state.fuel->get_scale().x - 0.0003f,
            g_game_state.fuel->get_scale().y,
            g_game_state.fuel->get_scale().z
        ));
    }
    else if (key_state[SDL_SCANCODE_RIGHT] && fuel_tank != 0)
    {
        g_game_state.player->move_right();
        g_game_state.player->set_rotate(-1.6f);

        g_game_state.rocket_fire->set_position(glm::vec3(
            g_game_state.player->get_position().x - 0.5f,
            g_game_state.player->get_position().y,
            g_game_state.player->get_position().z
        ));

        g_game_state.rocket_fire->set_rotate(-1.6f);
        g_game_state.rocket_fire->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

        fuel_tank -= fuel_usage;
        g_game_state.fuel->set_scale(glm::vec3(
            g_game_state.fuel->get_scale().x - 0.0003f,
            g_game_state.fuel->get_scale().y,
            g_game_state.fuel->get_scale().z
        ));


    }
    else {
        g_game_state.player->set_rotate(0.0f);

        g_game_state.rocket_fire->set_scale(glm::vec3(0.0f, 0.0f, 0.0f));
        g_game_state.rocket_fire->set_rotate(0.0f);
    }

    if (key_state[SDL_SCANCODE_UP] && fuel_tank != 0)
    {
        g_game_state.player->move_up();

        if (g_game_state.rocket_fire->get_rotate() == 0.0f) {
            g_game_state.rocket_fire->set_position(glm::vec3(
                g_game_state.player->get_position().x,
                g_game_state.player->get_position().y - 0.5f,
                g_game_state.player->get_position().z
            ));
        }

        g_game_state.rocket_fire->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

        fuel_tank -= fuel_usage;
        g_game_state.fuel->set_scale(glm::vec3(
            g_game_state.fuel->get_scale().x - 0.0003f,
            g_game_state.fuel->get_scale().y,
            g_game_state.fuel->get_scale().z
        ));
    }
    else {
        float x_acceleration = g_game_state.player->get_acceleration().x;
        g_game_state.player->set_acceleration(glm::vec3(x_acceleration, gravity, 0.0f));
    }
    

    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
    {
        g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
    }
}

void update()
{
    float collision_factor = 1;
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    g_game_state.player->update(delta_time);

    if (g_game_state.player->get_position().x < -5.5f || g_game_state.player->get_position().x > 5.5f) {
        g_game_state.text->set_scale(glm::vec3(9.0f, 2.0f, 0.0f));
        g_game_is_running = false;
    }

    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) {
        g_game_state.stone_block[i]->update(delta_time);
        if (g_game_state.stone_block[i]->check_collision(g_game_state.player)) {
            if (i == 8) {
                g_game_state.text->set_texture_id(load_texture(VICTORY_FILEPATH));
                g_game_state.text->set_scale(glm::vec3(10.0f, 2.0f, 0.0f));
                g_game_is_running = false;
            }
            else {
                g_game_state.text->set_scale(glm::vec3(9.0f, 2.0f, 0.0f));
                g_game_is_running = false;
            }
        }
    }

    g_game_state.land_indictor->update(delta_time);
    g_game_state.text->update(delta_time);
    g_game_state.rocket_fire->update(delta_time);
    g_game_state.fuel->update(delta_time);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.player->render(&g_shader_program);

    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) {
        g_game_state.stone_block[i]->render(&g_shader_program);
    }

    g_game_state.land_indictor->render(&g_shader_program);
    g_game_state.text->render(&g_shader_program);
    g_game_state.rocket_fire->render(&g_shader_program);
    g_game_state.fuel->render(&g_shader_program);

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

// ————— DRIVER GAME LOOP ————— /
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();

        render();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    shutdown();
    return 0;
}