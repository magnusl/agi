#include <agi/interpreter.h>
#include <agi/object.h>
#include <agi/view.h>

namespace agi {

unsigned GetPriorityY(uint8_t y)
{
    static const uint8_t levels[] = {48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168 };
    for(size_t i = 0; i < (sizeof(levels) / sizeof(uint8_t)); ++i) {
        if (y < levels[i]) {
            return i + 4;
        }
    }
    return 4;
}

/*****************************************************************************/
/*                                      Object                               */
/*****************************************************************************/

void Interpreter::AnimateObject(uint8_t id)
{
    auto& obj = GetObject(id);
    obj.flags = (ANIMATED_FLAG | UPDATE_FLAG | CYCLING_FLAG);
}

void Interpreter::UnanimateAll()
{
    for(auto& obj : objects_) {
        obj.flags &= ~(ANIMATED_FLAG | DRAWN_FLAG);
    }
}

void Interpreter::DrawObject(uint8_t id)
{
    auto& obj = GetObject(id);
    obj.flags |= (DRAWN_FLAG | UPDATE_FLAG);
}

void Interpreter::EraseObject(uint8_t id)
{
    auto& obj = GetObject(id);
    obj.flags &= ~DRAWN_FLAG;
}

void Interpreter::SetObjectPosition(uint8_t id, uint8_t x, uint8_t y)
{
    auto& obj = GetObject(id);
    obj.movement.x = x;
    obj.movement.y = y;
}

void Interpreter::GetObjectPosition(uint8_t id, uint8_t& x, uint8_t& y)
{
    auto& obj = GetObject(id);
    x = obj.movement.x;
    y = obj.movement.y;
}

void Interpreter::SetObjectView(uint8_t id, uint8_t view)
{
    auto& obj = GetObject(id);
    obj.animation.SetView(view, views_.GetView(view));
}

void Interpreter::Reposition(uint8_t id, uint8_t x, uint8_t y)
{
    auto& obj = GetObject(id);
    obj.movement.x += x;
    obj.movement.y += y;
}

void Interpreter::StartUpdate(uint8_t id)
{
    auto& obj = GetObject(id);
    obj.flags |= UPDATE_FLAG;
}

void Interpreter::StopUpdate(uint8_t id)
{
    auto& obj = GetObject(id);
    obj.flags &= ~UPDATE_FLAG;
}

void Interpreter::ForceUpdate(uint8_t)
{

}

void Interpreter::StartMotion(uint8_t id)
{
    auto& obj = GetObject(id);
    obj.movement.motion = Motion::kNormal;
    if (id == 0) {
        obj.movement.direction = Direction::kStationary;
        programControl_ = false;
    }
}

void Interpreter::StopMotion(uint8_t id)
{
    auto& obj = GetObject(id);
    obj.movement.motion = Motion::kNormal;
    obj.movement.direction = Direction::kStationary;
    if (id == 0) {
        programControl_ = true;
    }
}

void Interpreter::SetDirection(uint8_t id, uint8_t direction)
{
    if (direction < 9) {
        GetObject(id).movement.direction = static_cast<Direction>(direction);
    }
}

void Interpreter::MoveObject(
    uint8_t obj, uint8_t x, uint8_t y, uint8_t pixelPerStep, uint8_t flag)
{
    auto& object = GetObject(obj);
    object.movement.motion = Motion::kMoveObject;
    object.movement.moveObj.dstX = x;
    object.movement.moveObj.dstY = y;
    object.movement.moveObj.speed = pixelPerStep + 1;
    object.movement.moveObj.flag = flag;
}

uint8_t Interpreter::Distance(uint8_t, uint8_t)
{
    return 0xff;
}

/*****************************************************************************/
/*                                  Object updates                           */
/*****************************************************************************/
void Interpreter::UpdateDirections()
{
    /* For all objects for which command animate_obj, start_update 
       and draw were carried out, the recalculation of the direction
       of movement is performed. */
    for(auto& obj : objects_)
    {
        if ((obj.flags & (ANIMATED_FLAG | UPDATE_FLAG | DRAWN_FLAG)) ==
            (ANIMATED_FLAG | UPDATE_FLAG | DRAWN_FLAG))
        {
            UpdateDirections(obj);
        }
    }
}

namespace {

// direction of object 2 to object 1
Direction GetDirectionToObject(int x1, int y1, int x2, int y2)
{
    if (x1 < x2) {
        return (y1 < y2) ? Direction::kNorthWest :
            ((y1 == y2) ? Direction::kWest : Direction::kSouthWest);
    }
    else if (x1 > x2) {
        return (y1 < y2) ? Direction::kNorthEast :
            ((y1 == y2) ? Direction::kEast : Direction::kSouthEast);
    }
    else {
        return (y1 < y2) ? Direction::kNorth :
            ((y1 == y2) ? Direction::kStationary : Direction::kSouth);
    }
}

} // namespace

void Interpreter::UpdateDirections(Object& object)
{
    auto& movement = object.movement;
    switch(movement.motion) {
    case Motion::kNormal:
        break;
    case Motion::kWander:
        // set a random direction
        break;
    case Motion::kFollowEgo:
        break;
    case Motion::kMoveObject:
        // calculate the direction to the position
        object.movement.direction = GetDirectionToObject(
            object.movement.moveObj.dstX,
            object.movement.moveObj.dstY,
            object.movement.x,
            object.movement.y);
        break;
    default:
        break;
    }
}


static const int loopBasedOnDirection2[] = {
    4, 4, 0, 0, 0, 4, 1, 1, 1
};

static const int loopBasedOnDirection4[] = {
    4, 3, 0, 0, 0, 2, 1, 1, 1
};

void Interpreter::UpdateControlledObjects()
{
    for(auto& object : objects_) {

        if ((object.flags & (ANIMATED_FLAG | UPDATE_FLAG | DRAWN_FLAG)) !=
            (ANIMATED_FLAG | UPDATE_FLAG | DRAWN_FLAG))
        {
            continue;
        }

        if (~object.flags & FIXED_LOOP_FLAG) {
            // determine loop based on direction
            int loop = 4;
            switch(object.animation.numberOfLoops) {
            case 2:
            case 3:
                //loop = loopBasedOnDirection2[object.movement.direction];
                break;
            case 4:
                //loop = loopBasedOnDirection4[object.movement.direction];
                break;
            default:
                break;
            }
            if ((loop != 4) && (loop != object.animation.loopIndex)) {
                // set the new loop
            }
        }

        if (~object.flags & CYCLING_FLAG) {
            continue;
        }

        // check if we should update the animation this loop (cycleTime)
        AnimateObject(object);

        // draw the object
    }

    UpdatePositions();
}

void Interpreter::UpdatePositions()
{
    for(auto& object : objects_) {
        // check flags ANIMATED|UPDATE|DRAWN
    }
}

namespace {

void MoveObjectNormally(Object& object)
{
    static const int dx[] = {
        0, 0, 1, 1, 1, 0, -1, -1, -1
    };

    static const int dy[] = {
        0, -1, -1, 0, 1, 1, 1, 0, -1
    };

    auto& m = object.movement;
    size_t index = static_cast<size_t>(m.direction);
    int px = m.x + (dx[index] * m.stepSize);
    int py = m.y + (dy[index] * m.stepSize);

    m.x = px;
    m.y = py;
}

void MoveObjectToPoint(Object& object)
{
    auto& m = object.movement;
    float dx = static_cast<float>(m.moveObj.dstX) - static_cast<float>(m.x);
    float dy = static_cast<float>(m.moveObj.dstY) - static_cast<float>(m.y);
    float distance = sqrt(dx*dx + dy*dy);

    if (distance <= m.moveObj.speed) {
        // object reaches the destination in a single step
        m.x = m.moveObj.dstX;
        m.y = m.moveObj.dstY;
    }
    else {
        m.x += (static_cast<float>(dx) / distance) * m.moveObj.speed;
        m.y += (static_cast<float>(dy) / distance) * m.moveObj.speed;
    }
}

} // namespace

void Interpreter::UpdateMovement(Object& object)
{
    auto& m = object.movement;

    // don't move the object if it's stationary
    if (m.direction == Direction::kStationary) {
        return;
    }
    switch(m.motion) {
    case Motion::kNormal:
        MoveObjectNormally(object);
        break;
    case Motion::kWander:
        break;
    case Motion::kFollowEgo:
        break;
    case Motion::kMoveObject:
        MoveObjectToPoint(object);
        // set the flag since the destination is reached
        if ((m.x == m.moveObj.dstX) && (m.y == m.moveObj.dstY)) {
            m.motion = Motion::kNormal;
            flags_.set(m.moveObj.flag);                
        }
        break;
    default:
        break;
    }
}

void Interpreter::AnimationTick()
{
#if 0
    for(auto& animated : updating_) {
        // get the object and check if it's cycling
        auto& obj = GetObject(animated);
        if (obj.animation.viewInstance && obj.animation.cycling) {
            AnimateObject(obj);
        }
    }
#endif
    AnimateObject(GetObject(0));
}

namespace {

void SelectLoopFromDirection(Object& object)
{
    static const int changesLessThanFour[] = {
        -1, -1, 0, 0, 0, -1, 1, 1, 1
    };
    static const int changesFourOrMore[] = {
        -1, 3, 0, 0, 0, 2, 1, 1, 1
    };

    auto& m = object.movement;
    auto& anim = object.animation;
    int nextLoop = -1;
    if (anim.numberOfLoops >= 4) {
        // four or more loops
        nextLoop = changesFourOrMore[static_cast<size_t>(m.direction)];
    }
    else if (anim.numberOfLoops > 1) {
        nextLoop = changesLessThanFour[static_cast<size_t>(m.direction)];
    }  
    if ((nextLoop >= 0) && (anim.loopIndex != nextLoop)) {
        // change the animation loop
        anim.SetLoop(nextLoop);
    }
}

} // namespace

void Interpreter::AnimateObject(Object& object)
{
    auto& anim = object.animation;
    if (!anim.viewInstance) {
        return;
    }

#if 0
    // first check if we should change loop depending on direction
    if (!object.flags.loopFixed) {
        SelectLoopFromDirection(object);
    }
#endif

    switch(anim.cycleType) {
    case AnimationCycle::kNormal:
        // 0, 1, 2, ..., k-1, 0, 1, 2
        anim.celIndex = (anim.celIndex + 1) % anim.numberOfCels;
        break;
    case AnimationCycle::kEndOfLoop:
        if ((anim.celIndex + 1) >= anim.numberOfCels) {
            //anim.cycling = false;
            flags_.set(anim.flag);
        }
        else {
            ++anim.celIndex;
        }
        break;
    case AnimationCycle::kReverseLoop:
        if (anim.celIndex == 0) {
            //anim.cycling = false;
            flags_.set(anim.flag);
        }
        else {
            --anim.celIndex;
        }
        break;
    case AnimationCycle::kReverseCycle:
        if (anim.celIndex == 0) {
            anim.celIndex = anim.numberOfCels ? (anim.numberOfCels - 1) : 0;
        }
        else {
            --anim.celIndex;
        }
        break;
    default:
        assert(false);
        break;
    }
}

namespace {

void PaintCel(
    Framebuffer& framebuffer, const Cel& cel, size_t x, size_t y, uint8_t priority, size_t loopIndex)
{
    // for each pixel row in the cel
    int startY = y - cel.height + 1;
    for(uint8_t row = 0; row < cel.height; ++row) {
        // on which row should this line be rendered?
        int dstY = startY + row;
        if (dstY < 0) {
            // outside of the visible area, so skip this row
            continue;
        }
        // for each pixel in this row
        for(uint8_t col = 0; col < cel.width; ++col) {
            uint8_t pixel;
            // check if we should mirror the cel
            if (cel.mirrored && (cel.mirrorLoop != loopIndex)) {
                pixel = cel.pixels[(row * cel.width) + (cel.width - col - 1)];
            }
            else {
                pixel = cel.pixels[(row * cel.width) + col];
            }
            if (pixel != cel.colorKey) {
                // only draw the pixel if it's not transparent
                framebuffer.SetPixelIfHigherPriority(x + col, dstY, pixel, priority);
            }
        }
    }
}

void PaintObject(Framebuffer& framebuffer, Object& object)
{
    auto& animation = object.animation;
    if (!animation.viewInstance) {
        // no view instance, so skip
        return;
    }
    auto& loops = animation.viewInstance->loops;
    if (animation.loopIndex >= loops.size()) {
        // invalid loop index, skip
        return;
    }
    auto& loop = loops[animation.loopIndex];
    auto& cels = loop.cels;
    if (animation.celIndex >= cels.size()) {
        // invalid cel index, skip
        return;
    }
    // paint the cel
    PaintCel(
        framebuffer,
        cels[animation.celIndex],
        object.movement.x,
        object.movement.y,
        object.GetPriority(),
        animation.loopIndex);
}

} // namespace

void Interpreter::DrawObjects()
{

}

} // namespace agi
