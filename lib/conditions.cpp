#include <agi/interpreter.h>

namespace agi {

static const uint8_t ConditionArguments[] = {
    0, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 5, 1, 0, 0, 2, 5, 5, 5 
};

bool Interpreter::LogicalOr(ExecState& state)
{
    auto& code = state.script->code;
    uint8_t b = code[state.ip++];
    bool negation = false;
    bool ok = false;
    while(b != 0xfc) {
        if (b == 0xfd) {
            negation = !negation;
        }
        else {
            // result ^ negation
            if (ok = ProcessSingleCondition(b, state) != negation) {
                break;
            }
            negation = false;
        }
        b = code[state.ip++];
    }
    // if we left the loop early, consume bytes until 0xfc is encountered
    while(b != 0xfc) {
        b = code[state.ip++];
    }
    return ok;
}

bool Interpreter::LogicalAnd(ExecState& state)
{
    auto& code = state.script->code;
    uint8_t b = code[state.ip++];
    bool ok = true;
    bool negation = false;
    while((b != 0xff) && ok) {
        if (b == 0xfc) {
            ok = LogicalOr(state) != negation; // result ^ negation
            negation = false;
        }
        else if (b == 0xfd) {
            // inside negation
            negation = !negation;
        }
        else {
            // result ^ negation
            ok = ProcessSingleCondition(b, state) != negation;
            negation = false;
        }
        b = code[state.ip++];
    }
    // if we left the loop early, consume bytes until 0xff is encountered
    while(b != 0xff) {
        b = code[state.ip++];
    }
    return ok;
}

bool Interpreter::ProcessSingleCondition(uint8_t condition, ExecState& state)
{
    auto& code = state.script->code;
    if (condition == 0x0E) {
        // TODO: said()
        size_t count = code[state.ip++];
        state.ip += count * 2;
        return false;
    }
    else {
        size_t numberOfArguments = ConditionArguments[condition];
        if ((state.ip + numberOfArguments) >= code.size()) {
            throw std::runtime_error(
                "Condition arguments does not fit in the script area.");
        }
        switch(condition) {
        case 0x01:
            {
                // equaln
                size_t varIndex = code[state.ip++];
                uint8_t number = code[state.ip++];
                return variables_[varIndex] == number;
            }
        case 0x02:
            {
                // equalv
                size_t var1 = code[state.ip++];
                size_t var2 = code[state.ip++];
                return variables_[var1] == variables_[var2];
            }
        case 0x03:
            {
                // lessn (var, num)
                size_t varIndex = code[state.ip++];
                uint8_t number = code[state.ip++];
                return variables_[varIndex] < number;
            }
        case 0x04:
            {
                // lessv (var, var)
                size_t var1 = code[state.ip++];
                size_t var2 = code[state.ip++];
                return variables_[var1] < variables_[var2];
            }
        case 0x05:
            {
                // greatern (var, num)
                size_t varIndex = code[state.ip++];
                uint8_t number = code[state.ip++];
                return variables_[varIndex] > number;
            }
        case 0x06:
            {
                // greaterv (var, var)
                size_t var1 = code[state.ip++];
                size_t var2 = code[state.ip++];
                return variables_[var1] > variables_[var2];
            }
        case 0x07:
            {
                // isset (flag)
                size_t flag = code[state.ip++];
                return flags_.test(flag);
            }
        case 0x08:
            {
                // issetv (var)
                size_t flag = variables_[code[state.ip++]];
                return flags_.test(flag);   
            }
        case 0x09:
            {
                // TODO, has(n)
                uint8_t item = code[state.ip++];
                return false;
            }
        case 0x0a:
            {
                return false;
                /*


                uint8_t obj = code[state.ip++];
                uint8_t x1 = code[state.ip++];
                uint8_t y1 = code[state.ip++];
                uint8_t x2 = code[state.ip++];
                uint8_t y2 = code[state.ip++];
                return objects_.ObjectInBox(obj, x1, y1, x2, y2);

                */
            }
        case 0x0c:
            {
                // TODO: controller
                ++state.ip;
                return false;
            }
        case 0x0d:
            {
                return UserPressedKey();
            }
        default:
            return false;
            break;
        }
    }
}

} // namespace agi
