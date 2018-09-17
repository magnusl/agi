#include <agi/interpreter.h>

namespace agi {

namespace {

static const uint8_t ArgumentCount[] = {
    0, 1, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, // call.v
    1, 1, 0, 1, 1, 0, 1, 1, // var
    1, 1, 0, 1, 1, 3, 3, 3, // get.posn
    3, 2, 2, 2, 2, 1, 1, 2, // set.cel
    2, 2, 2, 2, 2, 2, 2, 2, // set.priority.v
    1, 2, 1, 1, 1, 1, 1, 1, // set.horizon
    1, 1, 1, 1, 1, 3, 1, 1, // start.cycling
    1, 2, 1, 2, 2, 1, 1, 2, // step.size
    2, 5, 5, 3, 1, 1, 2, 2, // get.dir
    1, 1, 4, 0, 1, 1, 1, 2, // put
    2, 2, 1, 2, 0, 1, 1, 3, // display
    3, 3, 0, 0, 1, 2, 1, 3, // configure.screen
    0, 0, 2, 5, 2, 1, 2, 0, // prevent.input
    0, 3, 7, 7, 0, 0, 0, 0, // init.disk
    0, 1, 3, 0, 0, 1, 1, 0, // show.mem
    0, 0, 0, 0, 0, 0, 1, 1, // set.game.id
    1, 0, 0, 3, 3, 0, 3, 4, // print.at
    4, 1, 5, 2, 1, 2, 0, 1, // enable.item
    1, 0, 1, 0, 0, 2, 2, 2, // div.n
    2, 0, 1, 0, 0, 0, 1, 1, // unknown175
    0, 1, 0, 4, 2, 0        // unknown 181
};

uint16_t GetU16(ExecState& state)
{
    auto& code = state.script->code;
    if ((state.ip + 2) < code.size()) {
        const uint8_t low = code[state.ip++]; // LSB
        const uint8_t high = code[state.ip++]; // MSB
        return (static_cast<uint16_t>(high) << 8) | low;
    }
    else {
        throw std::runtime_error(
            "Failed to read U16 since it's outside the valid script range.");
    }               
}

size_t GetNumberOfArguments(uint8_t cmd)
{
    assert(cmd < (sizeof(ArgumentCount) / sizeof(ArgumentCount[0])));
    return ArgumentCount[cmd];
}

} // namespace

boost::optional<UserActionRequest> Interpreter::Cycle()
{
    // get the script that we are currently executing
    boost::optional<UserActionRequest> uar;
    while(!scriptStack_.empty()) {
        auto& state = scriptStack_.back(); // get current script
        auto& code = state.script->code;
        assert(code.size() > 0);
        if (state.ip >= code.size()) {
            // reached the end of the script
            return boost::optional<UserActionRequest>();
        }

        // read the instruction byte
        const uint8_t cmd = code[state.ip++];
        if (cmd == 0xff) {
            state.ip += LogicalAnd(state) ? 2 : GetU16(state);
        }
        else if (cmd == 0xfe) {
            int16_t distance = GetU16(state);
            state.ip += distance;
        }
        else {
            const size_t argc = GetNumberOfArguments(cmd);
            if ((state.ip + argc) > code.size()) {
                throw std::runtime_error(
                    "Arguments to script command is outside of the script buffer.");
            }
            // the arguments follow the instruction
            const uint8_t* argv = argc ? &code[state.ip] : 0;
            // increment the instruction pointer
            state.ip += argc;
            // execute the command
            switch(GetCommandType(cmd)) {
            case CommandType::kArithmetic:
                ArithmeticCommand(cmd, argv);
                break;
            case CommandType::kResourceManagement:
                ResourceManagementCommand(cmd, argv);
                break;
            case CommandType::kProgramControl:
                ProgramControlCommand(cmd, argv);
                break;
            case CommandType::kObjectDescription:
                ObjectDescriptionCommand(cmd, argv);
                break;
            case CommandType::kObjectMotion:
                ObjectMotionCommand(cmd, argv);
                break;
            case CommandType::kInventoryItem:
                InventoryItemCommand(cmd, argv);
                break;
            case CommandType::kPictureManagement:
                PictureManagementCommand(cmd, argv);
                break;
            case CommandType::kSoundManagement:
                SoundManagementCommand(cmd, argv);
                break;
            case CommandType::kTextManagement:
                TextManagementCommand(cmd, argv);
                break;
            case CommandType::kStringManagement:
                uar = StringManagementCommand(cmd, argv);
                break;
            case CommandType::kInitialization:
                InitializationCommand(cmd, argv);
                break;
            case CommandType::kMenuManagement:
                MenuManagementCommand(cmd, argv);
                break;
            case CommandType::kOther:
                MiscCommand(cmd, argv);
                break;
            }
        }
        if (uar) {
            return uar;
        }
    }
    return uar;
}

/*****************************************************************************/
/*                              Arithmetic commands                          */
/*****************************************************************************/
void Interpreter::ArithmeticCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kIncrement:
        {
            // Var(n) = Var(n) + 1
            auto& var = variables_[arguments[0]];
            if (var < 255) {
                ++var;
            }
            break;
        }
    case ActionCommand::kDecrement:
        {
            // Var(n) = Var(n) - 1
            auto& var = variables_[arguments[0]];
            if (var > 0) {
                --var;
            }
            break;   
        }
    case ActionCommand::kAssignN:
        {
            // Var(n) = m
            SetVariable(arguments[0], arguments[1]);
            break;
        }
    case ActionCommand::kAssignV:
        {
            // Var(n) = Var(m)
            variables_[arguments[0]] = variables_[arguments[1]];
            break;
        }
    case ActionCommand::kAddN:
        {
            // Var(n) = Var(n) + m
            auto& var = variables_[arguments[0]];
            var = static_cast<uint8_t>(std::min(255u, static_cast<unsigned>(var) + arguments[1]));
            break;
        }
    case ActionCommand::kAddV:
        {
            // Var(n) = Var(n) + Var(m)
            auto& varN = variables_[arguments[0]];
            auto varM = variables_[arguments[1]];
            varN = static_cast<uint8_t>(std::min(255u, static_cast<unsigned>(varN) + static_cast<unsigned>(varM)));
            break;
        }
    case ActionCommand::kSubN:
        {
            // Var(n) = Var(n) - m
            auto& var = variables_[arguments[0]];
            auto m = arguments[1];
            var = (var < m) ? 0 : (var - m);
            break;   
        }
    case ActionCommand::kSubV:
        {
            // Var(n) = Var(n) - Var(m)
            auto& varN = variables_[arguments[0]];
            auto varM = variables_[arguments[1]];
            varN = (varN < varM) ? 0 : (varN - varM); 
            break;
        }
    case ActionCommand::kLIndirectV:
        {
            // Var(Var(n)) = Var(m)
            auto varN = variables_[arguments[0]];
            variables_[varN] = variables_[arguments[1]];
            break;
        }
    case ActionCommand::kRIndirect:
        {
            // Var(n) = Var(Var(m))
            auto varM = variables_[arguments[1]]; // Var(m)
            variables_[arguments[0]] = variables_[varM]; // Var(Var(m))
            break;
        }
    case ActionCommand::kLIndirectN:
        {
            // Var(Var(n)) = m
            auto varN = variables_[arguments[0]];
            variables_[varN] = arguments[1];
            break;
        }
    case ActionCommand::kSet:
        flags_.set(arguments[0]);
        break;
    case ActionCommand::kReset:
        flags_.reset(arguments[0]);
        break;
    case ActionCommand::kToggle:
        flags_.flip(arguments[0]);
        break;
    case ActionCommand::kSetV:
        flags_.set(variables_[arguments[0]]);
        break;
    case ActionCommand::kResetV:
        flags_.reset(variables_[arguments[0]]);
        break;
    case ActionCommand::kToggleV:
        flags_.flip(variables_[arguments[0]]);
        break;
    case ActionCommand::kRandom:
        // random(n, m, k)
        SetVariable(arguments[2], (rand() % arguments[1]) + arguments[0]);
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                  Resource management                      */
/*****************************************************************************/
void Interpreter::ResourceManagementCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kLoadLogics:
        scripts_.LoadScript(arguments[0]);
        break;
    case ActionCommand::kLoadLogicsV:
        scripts_.LoadScript(variables_[arguments[0]]);
        break;
    case ActionCommand::kLoadPic:
        break;
    case ActionCommand::kDiscardPic:
        break;
    case ActionCommand::kLoadView:
        break;
    case ActionCommand::kLoadViewV:
        break;
    case ActionCommand::kDiscardView:
        break;
    case ActionCommand::kDiscardViewV:
        break;
    case ActionCommand::kLoadSound:
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                  Program control                          */
/*****************************************************************************/

void Interpreter::ProgramControlCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kReturn:
        scriptStack_.pop_back();
        break;
    case ActionCommand::kCall:
        Call(arguments[0]);
        break;
    case ActionCommand::kCallV:
        Call(variables_[arguments[0]]);
        break;
    case ActionCommand::kNewRoom:
        return NewRoom(arguments[0]);
    case ActionCommand::kNewRoomV:
        return NewRoom(variables_[arguments[0]]);
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                Object description                         */
/*****************************************************************************/
void Interpreter::ObjectDescriptionCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kAnimateObj:
        AnimateObject(arguments[0]);
        break;
    case ActionCommand::kUnanimateAll:
        UnanimateAll();
        break;
    case ActionCommand::kDraw:
        DrawObject(arguments[0]);
        break;   
    case ActionCommand::kErase:
        EraseObject(arguments[0]);
        break;
    case ActionCommand::kPosition:
        SetObjectPosition(arguments[0], arguments[1], arguments[2]);
        break;
    case ActionCommand::kPositionV:
        SetObjectPosition(arguments[0], variables_[arguments[1]], variables_[arguments[2]]);
        break;
    case ActionCommand::kGetPosN:
        GetObjectPosition(arguments[1], variables_[arguments[1]], variables_[arguments[2]]);
        break;
    case ActionCommand::kSetView:
        SetObjectView(arguments[0], arguments[1]);
        break;   
    case ActionCommand::kSetViewV:
        SetObjectView(arguments[0], variables_[arguments[1]]);
        break;
    case ActionCommand::kSetLoop:
        GetObject(arguments[0]).animation.SetLoop(arguments[1]);
        break;
    case ActionCommand::kSetLoopV:
        GetObject(arguments[0]).animation.SetLoop(variables_[arguments[1]]);
        break;
    case ActionCommand::kFixLoop:
        GetObject(arguments[0]).flags |= FIXED_LOOP_FLAG;
        break;
    case ActionCommand::kReleaseLoop:
        GetObject(arguments[0]).flags &= ~FIXED_LOOP_FLAG;
        break;
    case ActionCommand::kSetCel:
        GetObject(arguments[0]).animation.SetCel(arguments[1]);
        break;
    case ActionCommand::kSetCelV:
        GetObject(arguments[0]).animation.SetCel(variables_[arguments[1]]);
        break;
    case ActionCommand::kLastCel:
        variables_[arguments[1]] = GetObject(arguments[0]).animation.LastCel();
        break;
    case ActionCommand::kCurrentCel:
        variables_[arguments[1]] = GetObject(arguments[0]).animation.celIndex;
        break;
    case ActionCommand::kCurrentLoop:
        variables_[arguments[1]] = GetObject(arguments[0]).animation.loopIndex;
        break;
    case ActionCommand::kCurrentView:
        variables_[arguments[1]] = GetObject(arguments[0]).animation.viewIndex;
        break;
    case ActionCommand::kNumberOfLoops:
        variables_[arguments[1]] = GetObject(arguments[0]).animation.numberOfLoops;
        break;
    case ActionCommand::kSetPriority:
        GetObject(arguments[0]).animation.priority = arguments[1];
        break;
    case ActionCommand::kSetPriorityV:
        GetObject(arguments[0]).animation.priority = variables_[arguments[1]];
        break;
    case ActionCommand::kReleasePriority:
        GetObject(arguments[0]).flags &= ~FIXED_PRIORITY_FLAG;
        break;
    case ActionCommand::kGetPriority:
        variables_[arguments[1]] = GetObject(arguments[0]).GetPriority();
        break;
    case ActionCommand::kStopCycling:
        GetObject(arguments[0]).animation.StopCycling();
        break;
    case ActionCommand::kStartCycling:
        GetObject(arguments[0]).animation.StartCycling();
        break;
    case ActionCommand::kNormalCycle:
        GetObject(arguments[0]).animation.NormalCycle();
        break;
    case ActionCommand::kEndOfLoop:
        GetObject(arguments[0]).animation.EndOfLoop(arguments[1]);
        break;
    case ActionCommand::kReverseCycle:
        GetObject(arguments[0]).animation.ReverseCycle();
        break;
    case ActionCommand::kReverseLoop:
        GetObject(arguments[0]).animation.ReverseLoop(arguments[1]);
        break;
    case ActionCommand::kCycleTime:
        GetObject(arguments[0]).animation.cycleTime = arguments[1];
        break;
    default:
        assert(false);
    }
}
/*****************************************************************************/
/*                                  Object motion                            */
/*****************************************************************************/
void Interpreter::ObjectMotionCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kReposition:
        Reposition(arguments[0], variables_[arguments[1]], variables_[arguments[2]]);
        break;
    case ActionCommand::kStopUpdate:
        StopUpdate(arguments[0]);
        break;
    case ActionCommand::kStartUpdate:
        StartUpdate(arguments[0]);
        break;
    case ActionCommand::kForceUpdate:
        ForceUpdate(arguments[0]);
        break;
    case ActionCommand::kIgnoreHorizon:
        GetObject(arguments[0]).flags &= ~OBSERVE_HORIZON_FLAG;
        break;
    case ActionCommand::kObserveHorizon:
        GetObject(arguments[0]).flags &= OBSERVE_HORIZON_FLAG;
        break;
    case ActionCommand::kSetHorizon:
        horizon_ = arguments[0];
        break;
    case ActionCommand::kObjectOnWater:
        GetObject(arguments[0]).movement.allowedSurface = SurfaceType::kWater;
        break;
    case ActionCommand::kObjectOnLand:
        GetObject(arguments[0]).movement.allowedSurface = SurfaceType::kLand;
        break;
    case ActionCommand::kObjectOnAnything:
        GetObject(arguments[0]).movement.allowedSurface = SurfaceType::kAny;
        break;
    case ActionCommand::kIgnoreObjects:
        GetObject(arguments[0]).flags &= ~OBSERVE_OBJECTS_FLAG;
        break;
    case ActionCommand::kObserveObjects:
        GetObject(arguments[0]).flags |= OBSERVE_OBJECTS_FLAG;
        break;
    case ActionCommand::kDistance:
        variables_[arguments[2]] = Distance(arguments[0], arguments[1]);
        break;
    case ActionCommand::kStopMotion:
        StopMotion(arguments[0]);
        break;
    case ActionCommand::kStartMotion:
        StartMotion(arguments[0]);
        break;
    case ActionCommand::kStepSize:
        GetObject(arguments[0]).movement.stepSize = variables_[arguments[1]];
        break;
    case ActionCommand::kStepTime:
        GetObject(arguments[0]).movement.stepTime = variables_[arguments[1]];
        break;
    case ActionCommand::kMoveObj:
        MoveObject(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
        break;
    case ActionCommand::kIgnoreBlocks:
        //assert(false);
        break;
    case ActionCommand::kProgramControl:
        programControl_ = true;
        break;
    case ActionCommand::kObserveBlocks:
        //assert(false);
        break;
    case ActionCommand::kPlayerControl:
        programControl_ = false;
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                Picture management                         */
/*****************************************************************************/

void Interpreter::PictureManagementCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kDrawPic:
        pictures_.DrawPicture(variables_[arguments[0]], pictureBuffer_);
        break;
    case ActionCommand::kShowPic:
        ShowPic();
        break;
    case ActionCommand::kOverlayPic:
        OverlayPic(variables_[arguments[0]]);
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                   Text management                         */
/*****************************************************************************/
void Interpreter::TextManagementCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kDisplay:
        //DisplayMessage(arguments[0], arguments[1], arguments[2], script);
        break;
    case ActionCommand::kDisplayV:
        // TODO

#if 0       
            DisplayMessage(
            variables_[arguments[0]],
            variables_[arguments[1]],
            variables_[arguments[2]],
            script);
#endif
        break;
    case ActionCommand::kTextscreen:
        break;
    case ActionCommand::kSetCursorChar:     // 0x6C
        // TODO
        break;
    case ActionCommand::kPreventInput:
        break;
    case ActionCommand::kClearLines:
        pictureBuffer_.ClearLines(arguments[0], arguments[1], arguments[2]);
        break;
    case ActionCommand::kGraphics:
        break;
    case ActionCommand::kStatusLineOn:
        break;
    case ActionCommand::kAcceptInput:
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                     Misc commands                         */
/*****************************************************************************/
void Interpreter::MiscCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kShowPriScreen:
        ShowPriorityScreen();
        break;
    case ActionCommand::kConfigureScreen:
        // TODO ConfigureScreen
        break;
    case ActionCommand::kOpenDialogue:
    case ActionCommand::kCloseDialogue:
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                   String management                       */
/*****************************************************************************/
boost::optional<UserActionRequest> Interpreter::StringManagementCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kSetString:
        break;
    case ActionCommand::kGetstring:
        // (n, m, x, y, l)
        /*DisplayMessage(
            arguments[2],
            arguments[3],
            arguments[1],
            script);*/
        break;
    case ActionCommand::kParse:
        break;
    default:
        assert(false);
    }
    return boost::optional<UserActionRequest>();
}

/*****************************************************************************/
/*                                    Initialization                         */
/*****************************************************************************/
void Interpreter::InitializationCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kSetKey:
        break;
    case ActionCommand::kSetGameId:
        break;
    case ActionCommand::kTraceInfo:
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                    Menu management                        */
/*****************************************************************************/
void Interpreter::MenuManagementCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kDisableItem:
        break;
    case ActionCommand::kSetMenu:       // 0x9c
        break;
    case ActionCommand::kSetMenuItem:   // 0x9d
        break;
    case ActionCommand::kSubmitMenu:    // 0x9e
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                    Sound management                       */
/*****************************************************************************/
void Interpreter::SoundManagementCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    case ActionCommand::kSound:
    case ActionCommand::kStopsound:
    case ActionCommand::kLoadSound:
        break;
    default:
        assert(false);
    }
}

/*****************************************************************************/
/*                                       Inventory                           */
/*****************************************************************************/
void Interpreter::InventoryItemCommand(
    uint8_t cmd,
    const uint8_t* arguments)
{
    switch(static_cast<ActionCommand>(cmd)) {
    default:
        assert(false);
    }
}

} // namespace agi
