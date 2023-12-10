#ifndef INPUT_H
#define INPUT_H
#include "glfw/glfw3.h"
#include "game_types.h"
#include <iostream>

class Input
{
public:
    Input();

    static inline void registerHeld(int key, int inputBitFlag) {
        if (inputBitFlag >= 0 && inputBitFlag < (sizeof(input_t) * 8)) {
            keys[inputBitFlag] = key;
            std::cout << "registerd " << key << " " << inputBitFlag << std::endl;
        }
    }

    static inline input_t getHeld() {
        return held;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
//        std::cout << "callback " << key << " " << action << std::endl;
        for (int i = 0; i < keys.size(); i++) {
            if (keys[i] == 0 || key != keys[i])
                continue;

            int state = action;

            input_t held_change = -1;
            held_change = held_change & (1 << i);

            if (state == GLFW_PRESS){
                held |= held_change;
            } else if (state == GLFW_RELEASE) {
                held &= ~(1 << i);
            }
            std::cout << (int)held << std::endl;
        }
    }

    static inline bool isHeld(int inputBitFlag) {
        return (held & (1 << inputBitFlag));
    }

    static inline bool isHeld(input_t in, int inputBitFlag) {
        return (in & (1 << inputBitFlag));
    }

//    static void checkKeys(GLFWwindow* window) {

//        for (int i = 0; i < keys.size(); i++) {
//            if (keys[i] == 0)
//                continue;

//            int state = glfwGetKey(window, keys[i]);
//            input_t held_change = -1;
//            held_change = held_change & (1 << i);

//            if (state == GLFW_PRESS){
//                held &= held_change;
//            } else if (state == GLFW_RELEASE) {
//                held &= ~(1 << i);
//            }
//        }

//    }
private:
    // input bit -> key int
    static inline std::array<int, sizeof(input_t) * 8> keys{};
    static inline input_t held = 0;

};

#endif // INPUT_H
