#include <agi/interpreter.h>
#include <boost/optional.hpp>

namespace agi {


void Interpreter::ClearKeyboardBuffer()
{

}

bool Interpreter::UserPressedKey()
{
    return GetVariable(Variable::kPressedKey) != 0;
}

void Interpreter::PollInput()
{
    std::bitset<SDL_NUM_SCANCODES> pressedKeys;
    // process all the key events
    for(auto& keySym : keys_) {
        pressedKeys.set(keySym.scancode);
    }
    keys_.clear();

    if (programControl_) {
        // assign ego the direction of var(6)
        SetDirection(0, GetVariable(Variable::kEgoDirection));
    }
    else {
        auto& ego = GetObject(0).movement;
        // poll input
        if (pressedKeys.test(SDL_SCANCODE_LEFT)) {
            ego.direction =
                (ego.direction == Direction::kWest) ? Direction::kStationary : Direction::kWest;
        }
        else if (pressedKeys.test(SDL_SCANCODE_UP)) {
            ego.direction =
                (ego.direction == Direction::kNorth) ? Direction::kStationary : Direction::kNorth;   
        }
        else if (pressedKeys.test(SDL_SCANCODE_RIGHT)) {
            ego.direction =
                (ego.direction == Direction::kEast) ? Direction::kStationary : Direction::kEast;
        }
        else if (pressedKeys.test(SDL_SCANCODE_DOWN)) {
            ego.direction =
                (ego.direction == Direction::kSouth) ? Direction::kStationary : Direction::kSouth;
        }
        // now set variable(6) to reflect the motion
        SetVariable(Variable::kEgoDirection, static_cast<uint8_t>(ego.direction));
    }
}

} // namespace agi
