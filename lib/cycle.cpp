#include <agi/interpreter.h>

namespace agi {

boost::optional<UserActionRequest> Interpreter::StartCycle()
{
    // Flag (2) - > 0
    // Flag (4) - > 0
    SetFlag(Flag::kPlayerCommandEntered, false);
    SetFlag(Flag::kUserInputAccepted, false);

    // 3. poll keyboard and joystick
    PollInput();

    // Update direction of EGO
    if (programControl_) {

    }
    else {

    }

    /* For all objects for which command animate_obj, start_update 
       and draw were carried out, the recalculation of the direction
       of movement is performed. */
    UpdateDirections();

    // 4 Logic 0 is executed
    scriptStack_.clear();
    Call(0);

    // execute the actual script
    return ResumeCycle();
}

boost::optional<UserActionRequest> Interpreter::ResumeCycle()
{
    // update the clock variables
    UpdateClock();
    auto uar = Cycle();
    if (!uar) {
        // no new user action request, so finish the current cycle
        FinishCycle();
    }
    return uar;
}

void Interpreter::FinishCycle()
{
    // Logic0 has been executed
    SetFlag(Flag::kFirstLogic0Execution, false);

    // Dir. of motion of EGO  <-- var (6)

    // Reset the variables that indicate if any other object touched the border
    SetVariable(Variable::kObjectTouchCode, 0);
    SetVariable(Variable::kObjectTouchCode, 0);    

    // Reset some flags
    SetFlag(Flag::kRestartCmdExecuted, false);
    SetFlag(Flag::kRestoreGameExecuted, false);

    // PaintScene
    PaintScene();

    // now update any controlled objects
    UpdateControlledObjects();
}

void Interpreter::PaintScene()
{
    framebuffer_ = pictureBuffer_;
}

} // namespace agi
