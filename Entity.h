/**
* Author: Gianfranco Romani
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

class Entity
{
private:
    int* m_animation_right = NULL, // move to the right
        * m_animation_left = NULL, // move to the left
        * m_animation_up = NULL, // move upwards
        * m_animation_down = NULL; // move downwards

    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::mat4 m_model_matrix;
    glm::vec3 m_scale;
    float rotate;

    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

public:
    // ————— STATIC VARIABLES ————— //
    static const int SECONDS_PER_FRAME = 4;
    static const int LEFT = 0,
        RIGHT = 1,
        UP = 2,
        DOWN = 3;

    // ————— ANIMATION ————— //
    int** m_walking = new int* [4]
    {
        m_animation_left,
            m_animation_right,
            m_animation_up,
            m_animation_down
    };

    int m_animation_frames = 0,
        m_animation_index = 0,
        m_animation_cols = 0,
        m_animation_rows = 0;

    int* m_animation_indices = NULL;
    float m_animation_time = 0.0f;

    // ————— METHODS ————— //
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void update(float delta_time);
    void render(ShaderProgram* program);

    void move_left() { m_acceleration.x = -0.75f; };
    void move_right() { m_acceleration.x = 0.75f; };
    void move_up() { m_acceleration.y = 0.5f; };

    bool const check_collision(Entity* other) const;

    // ————— GETTERS ————— //
    glm::vec3 const get_position()   const { return m_position; };
    glm::vec3 const get_scale()   const { return m_scale; };
    glm::vec3 const get_movement()   const { return m_movement; };
    GLuint    const get_texture_id() const { return m_texture_id; };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    float const get_rotate() const { return rotate; };

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; };
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
    void const set_rotate(float new_rotate) { rotate = new_rotate; };
};
