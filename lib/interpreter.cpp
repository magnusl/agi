#include <agi/interpreter.h>
#include <agi/commands.h>
#include <agi/util.h>
#include <stack>
#include <chrono>
#include <assert.h>
#include <boost/optional.hpp>

namespace agi {

Interpreter::Interpreter(const boost::filesystem::path& path) :
    volumes_(path),
    scripts_(volumes_, path / "LOGDIR"),
    views_(volumes_, path / "VIEWDIR"),
    pictures_(volumes_, path / "PICDIR")
{
    // set all the variables to zero
    std::fill(variables_.begin(), variables_.end(), 0);
    SetInitialState();
}

void Interpreter::SetInitialState()
{
    // set variables
    SetVariable(Variable::kCycleDelayTime, 1);      // 1/20 cycle delay
    SetVariable(Variable::kFreeMemoryPages, 255);   // 255 free memory pages
    SetVariable(Variable::kInputBufferSize, 41);    // input buffer size is 41 characters
    SetVariable(Variable::kComputerType, 0);        // IBM-PC
    SetVariable(Variable::kSoundType, 0);           // IBM-PC
    SetVariable(Variable::kMonitorType, 3);         // EGA
    UpdateClock();                                  // set clock variables

    // set flags
    SetFlag(Flag::kFirstLogic0Execution, true);             // first time LOGIC.0 is executed

    // set initial control mode
    programControl_ = true;
    SetFlag(Flag::kRoomScriptExecutedForFirstTime, true);
    SetVariable(Variable::kPressedKey, 0);
}

unsigned Interpreter::GetCycleDelay() const noexcept
{
    return GetVariable(Variable::kCycleDelayTime) * 50;
}

void Interpreter::OnKeyPress(SDL_Keysym keySym)
{
    keys_.push_back(keySym);
    SetVariable(Variable::kPressedKey, keySym.scancode);
}

void Interpreter::UpdateClock()
{
    // update the variables that indicates the interpreters internal clock.
    auto now = std::chrono::system_clock::now();
    std::time_t tm = std::chrono::system_clock::to_time_t(now);
    if (auto lm = std::localtime(&tm)) {
        SetVariable(Variable::kClockSeconds, lm->tm_sec);
        SetVariable(Variable::kClockMinutes, lm->tm_min);
        SetVariable(Variable::kClockHours, lm->tm_hour);
        SetVariable(Variable::kClockDay, lm->tm_mday);
    }
}

void Interpreter::NewRoom(uint8_t room)
{
    // - stop.update
    // - unanimate.all;
    UnanimateAll();

    // - player.control;
    // TODO: PlayerControl();
    // - unblock;
    // - set_horizon 36;
    horizon_ = 36;
    // - var (1) = var (0);
    SetVariable(Variable::kPreviousRoom, GetVariable(Variable::kCurrentRoom));
    // - var (0) = n | Var(n); 
    SetVariable(Variable::kCurrentRoom, room);
    // - var (4) = 0;
    SetVariable(Variable::kObjectThatTouchedBorder, 0);
    // - var (5) = 0; 
    SetVariable(Variable::kObjectTouchCode, 0);
    // - var (9) = 0;
    SetVariable(Variable::kMismatchedWords, 9);
    // var (16) = number of view assoc. w/Ego;
    // Ego coords from var (2);

    // var (2) = 0;
    SetVariable(Variable::kEgoTouchCode, 0);

    // - flag (5) - > 1!!!!
    SetVariable(Variable::kObjectTouchCode, 1);
    // - score < - var (3);
    // clear the script stack to terminate the cycle
    scriptStack_.clear();
}

void Interpreter::Call(uint8_t logicNumber)
{
    SetFlag(Flag::kRoomScriptExecutedForFirstTime, !roomFlags_.test(logicNumber));
    roomFlags_.set(logicNumber);
    scriptStack_.emplace_back(scripts_.GetScript(logicNumber));
}

void Interpreter::ShowPic()
{
}

void Interpreter::OverlayPic(uint8_t pictureNumber)
{

}

void Interpreter::ShowPriorityScreen()
{
    
}

void Interpreter::DisplayMessage(uint8_t row, uint8_t col, uint8_t msgIndex, const Script& script)
{
    if (0 == msgIndex) {
        return;
    }
    if (const char* pmsg = script.messages.at(msgIndex - 1)) {
        pictureBuffer_.Display(row, col, pmsg);
    }
}

} // namespace agi