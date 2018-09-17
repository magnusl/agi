#pragma once

#include <agi/commands.h>
#include <agi/array_view.h>
#include <agi/object_table.h>
#include <agi/script_loader.h>
#include <agi/volume_loader.h>
#include <agi/picture_loader.h>
#include <agi/view_loader.h>
#include <agi/framebuffer.h>
#include <agi/uar.h>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <bitset>
#include <array>
#include <stdint.h>
#include <iostream>
#include <SDL.h>

namespace agi {

struct ExecState {
    ExecState(std::shared_ptr<Script>&& code) :
        script(code),
        ip(0)
    {
        // empty 
    }

    std::shared_ptr<Script> script;
    size_t ip;                          // instruction pointer in script       
};

enum class ControlMode {
    kProgramControl,
    kPlayerControl
};

enum Variable {
    kCurrentRoom                = 0,
    kPreviousRoom               = 1,
    kEgoTouchCode               = 2,
    kScore                      = 3,
    kObjectThatTouchedBorder    = 4,
    kObjectTouchCode            = 5,
    kEgoDirection               = 6,
    kMaxScore                   = 7,
    kFreeMemoryPages            = 8,
    kMismatchedWords            = 9,
    kCycleDelayTime             = 10,
    kClockSeconds               = 11,
    kClockMinutes               = 12,
    kClockHours                 = 13,
    kClockDay                   = 14,
    kJoystickSensitivity        = 15,
    kEgoViewResource            = 16,
    kErrorCode                  = 17,
    kAdditionalErrorInfo        = 18,
    kPressedKey                 = 19,
    kComputerType               = 20,
    kMessageWindowTimer         = 21,
    kSoundType                  = 22,
    kSoundVolume                = 23,
    kInputBufferSize            = 24,
    kSelectedInventoryItem      = 25,
    kMonitorType                = 26
};

enum class Flag {
    kEgoOnWater                     = 0,
    kEgoObscured                    = 1,
    kPlayerCommandEntered           = 2,
    kEgoTouchedTrigger              = 3,
    kUserInputAccepted              = 4,
    kRoomScriptExecutedForFirstTime = 5,
    kRestartCmdExecuted             = 6,
    kScriptBufferBlocked            = 7,
    kCustomJoystickSensitivity      = 8,
    kSoundEnabled                   = 9,
    kDebuggerEnabled                = 10,
    kFirstLogic0Execution           = 11,
    kRestoreGameExecuted            = 12,
    kStatusSelectItems              = 13,
    kEnableMenu                     = 14,
    kEnableNonBlockingWindows       = 15
};

/**
 * \class   Interpreter
 */
class Interpreter
{
public:
    /**
     * \brief   Constructor
     */
    Interpreter(const boost::filesystem::path& path);

    /**
     * \brief   Returns the framebuffer associated with the interpreter
     */
    Framebuffer& GetFramebuffer() { return framebuffer_; }


    boost::optional<UserActionRequest> StartCycle();
    boost::optional<UserActionRequest> ResumeCycle();

    unsigned GetCycleDelay() const noexcept;

    /**
     * \brief   Called when a key is pressed
     */
    void OnKeyPress(SDL_Keysym);

protected:
    boost::optional<UserActionRequest> Cycle();
    void FinishCycle();
    void PaintScene();

    void UpdateDirections();
    void UpdateDirections(Object&);
    void UpdateControlledObjects();
    void DrawObjects();
    void AnimationTick();
    void AnimateObject(Object& object);
    void UpdateMovements();
    void UpdateMovement(Object& object);
    void UpdatePositions();

    /*************************************************************************/
    /*                                  Object management                    */
    /*************************************************************************/
    Object& GetObject(uint8_t index) { return objects_[index]; }

    void AnimateObject(uint8_t);
    void UnanimateAll();
    void DrawObject(uint8_t);
    void EraseObject(uint8_t);
    void SetObjectPosition(uint8_t id, uint8_t x, uint8_t y);
    void GetObjectPosition(uint8_t id, uint8_t& x, uint8_t& y);
    void SetObjectView(uint8_t id, uint8_t view);
    void Reposition(uint8_t id, uint8_t x, uint8_t y);
    void StartUpdate(uint8_t);
    void StopUpdate(uint8_t);
    void ForceUpdate(uint8_t);
    void StartMotion(uint8_t);
    void StopMotion(uint8_t);
    void SetDirection(uint8_t id, uint8_t dir);
    void MoveObject(uint8_t obj, uint8_t x, uint8_t y, uint8_t pixelPerStep, uint8_t flag);
    uint8_t Distance(uint8_t, uint8_t);

    /*************************************************************************/
    /*                              Command handlers                         */
    /*************************************************************************/
    void ArithmeticCommand(uint8_t cmd, const uint8_t* arguments);
    void ResourceManagementCommand(uint8_t cmd, const uint8_t* arguments);
    void ProgramControlCommand(uint8_t cmd, const uint8_t* arguments);
    void ObjectDescriptionCommand(uint8_t cmd, const uint8_t* arguments);
    void ObjectMotionCommand(uint8_t cmd, const uint8_t* arguments);
    void InventoryItemCommand(uint8_t cmd, const uint8_t* arguments);
    void PictureManagementCommand(uint8_t cmd, const uint8_t* arguments);
    void SoundManagementCommand(uint8_t cmd, const uint8_t* arguments);
    void TextManagementCommand(uint8_t cmd, const uint8_t* arguments);
    void InitializationCommand(uint8_t cmd, const uint8_t* arguments);
    void MenuManagementCommand(uint8_t cmd, const uint8_t* arguments);
    void MiscCommand(uint8_t cmd, const uint8_t* arguments);
    boost::optional<UserActionRequest> StringManagementCommand(
        uint8_t cmd, const uint8_t* arguments);

    void SetInitialState();
    void UpdateClock();
    void Execute(std::shared_ptr<Script>);
    void ClearKeyboardBuffer();
    void PollInput();
    void ExecuteCommand(
        const Script&, ActionCommand cmd, const uint8_t* arguments, size_t argumentCount);

    bool LogicalAnd(ExecState& state);
    bool LogicalOr(ExecState& state);
    bool ProcessSingleCondition(uint8_t condition, ExecState& state);

    void NewRoom(uint8_t room);
    void Call(uint8_t logicNumber);
    
    void ShowPic();
    void OverlayPic(uint8_t pictureNumber);
    void ShowPriorityScreen();
    bool UserPressedKey();
    /*************************************************************************/
    /*                                  Resource loading                     */
    /*************************************************************************/

    inline void SetVariable(uint8_t var, uint8_t value) {
        variables_[static_cast<size_t>(var)] = value;
    }

    inline uint8_t GetVariable(Variable var) const noexcept {
        return variables_[static_cast<size_t>(var)];
    }

    inline void SetFlag(Flag flag, bool value) {
        flags_[static_cast<size_t>(flag)] = value ? 1 : 0;
    }

    inline bool GetFlag(Flag flag) const noexcept {
        return flags_[static_cast<size_t>(flag)] ? true : false;
    }

    void DisplayMessage(
        uint8_t row, uint8_t col, uint8_t message, const Script& script);
private:
    /*************************************************************************/
    /*                                  Loaders                              */
    /*************************************************************************/
    VolumeLoader volumes_;
    ScriptLoader scripts_;
    PictureLoader pictures_;
    ViewLoader views_;
    Framebuffer pictureBuffer_;
    Framebuffer framebuffer_;
    std::vector<ExecState> scriptStack_;

    std::vector<SDL_Keysym> keys_;
    std::bitset<256> flags_;
    std::bitset<256> roomFlags_;
    std::array<uint8_t, 256> variables_;
    std::array<Object, 256> objects_;
    uint8_t horizon_;
    bool programControl_ = true;
};

} // namespace agi

