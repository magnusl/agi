#pragma once

#include <agi/view.h>
#include <memory>
#include <stdint.h>

namespace agi {

/**
 * \brief   Get priority based on Y coordinate
 */
unsigned GetPriorityY(uint8_t y);

/**
 * \enum    Direction
 */
enum class Direction
{
    kStationary = 0,
    kNorth      = 1,
    kNorthEast  = 2,
    kEast       = 3,
    kSouthEast  = 4,
    kSouth      = 5,
    kSouthWest  = 6,
    kWest       = 7,
    kNorthWest  = 8
};

/**
 * \enum    AnimationCycle    
 */
enum class AnimationCycle
{
    kNormal,
    kEndOfLoop,
    kReverseLoop,
    kReverseCycle
};

/**
 * \enum  Motion
 */
enum class Motion
{
    kNormal,
    kWander,
    kFollowEgo,
    kMoveObject
};

/**
 * \enum    SurfaceType
 */
enum class SurfaceType {
    kAny,
    kWater,
    kLand
};

/** 
 * \struct  Animation
 */
struct Animation
{
    Animation() :
        viewIndex(0),
        loopIndex(0),
        celIndex(0),
        cycleTime(1),
        numberOfLoops(0),
        numberOfCels(0),
        flag(0),
        priority(15),
        cycleType(AnimationCycle::kNormal)
    {
        // empty
    }

    void SetView(uint8_t, std::shared_ptr<View>&& view);
    void SetLoop(uint8_t);
    void SetCel(uint8_t);
    void StartCycling();
    void StopCycling();
    void NormalCycle();
    void EndOfLoop(uint8_t);
    void ReverseCycle();
    void ReverseLoop(uint8_t);

    uint8_t LastCel() const;

    std::shared_ptr<View> viewInstance;
    uint8_t viewIndex;
    uint8_t loopIndex;
    uint8_t celIndex;
    uint8_t cycleTime;
    uint8_t numberOfLoops;
    uint8_t numberOfCels;
    uint8_t flag;
    uint8_t priority;
    AnimationCycle cycleType;
};

/**
 * \struct  MoveObject
 */
struct MoveObject
{
    MoveObject() :
        dstX(0),
        dstY(0),
        speed(0),
        flag(0)
    {

    }
    uint8_t dstX;
    uint8_t dstY;
    uint8_t speed;
    uint8_t flag;
};

/**
 * \struct  Movement
 */
struct Movement
{
    Movement() :
        x(0),
        y(0),
        xSize(0),
        ySize(0),
        direction(Direction::kStationary),
        motion(Motion::kNormal),
        allowedSurface(SurfaceType::kAny),
        stepSize(1),
        stepTime(1)
    {
        // empty
    }

    int x;
    int y;
    uint8_t xSize;
    uint8_t ySize;
    Direction direction;
    Motion motion;
    SurfaceType allowedSurface;
    MoveObject moveObj;
    uint8_t stepSize;
    uint8_t stepTime;
};

enum {
    ANIMATED_FLAG           = (1 << 0),
    UPDATE_FLAG             = (1 << 1),
    DRAWN_FLAG              = (1 << 2),
    CYCLING_FLAG            = (1 << 3),
    OBSERVE_BLOCKS_FLAG     = (1 << 4),
    FIXED_PRIORITY_FLAG     = (1 << 5),
    OBSERVE_HORIZON_FLAG    = (1 << 6),
    VIEW_ON_WATER_FLAG      = (1 << 7),
    VIEW_ON_LAND_FLAG       = (1 << 8),
    FIXED_LOOP_FLAG         = (1 << 9),
    OBSERVE_OBJECTS_FLAG    = (1 << 10)
};

/**
 * \struct  Object
 */
struct Object
{
    Movement movement;
    Animation animation;
    uint32_t flags = 0;

    uint8_t GetPriority() const {
        return (flags & FIXED_PRIORITY_FLAG) ? animation.priority : GetPriorityY(movement.y);
    }
};

} // namespace agi