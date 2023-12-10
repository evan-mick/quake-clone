#ifndef INPUT_H
#define INPUT_H
#include "glfw/glfw3.h"
#include "game_types.h"

class Input
{
public:
    Input();

    static inline void registerHeld(int key, int inputBitFlag) {
        if (inputBitFlag >= 0 && inputBitFlag < sizeof(input_t))
            keys[inputBitFlag] = key;
    }

    static inline input_t getHeld() {
        return held;
    }

    static void checkKeys(GLFWwindow* window) {

        for (int i = 0; i < keys.size(); i++) {
            if (keys[i] == 0)
                continue;

            int state = glfwGetKey(window, keys[i]);
            input_t held_change = -1;
            held_change = held_change & (1 << i);

            if (state == GLFW_PRESS)
            {
                held &= held_change;

            } else if (state == GLFW_RELEASE) {
                held &= ~(i << i);
            }
        }

    }
private:
    // input bit -> key int
    static inline std::array<int, sizeof(input_t) * 8> keys{};
    static inline input_t held = 0;

};

#endif // INPUT_H
