#include <agi/object_table.h>
#include <string>
#include <algorithm>

namespace agi {

ObjectTable::ObjectTable(ViewLoader& views, std::bitset<256>&flags) :
    views_(views),
    flags_(flags)
{
    // empty
}

void ObjectTable::UpdateObjects()
{
    // for each object that is controlled by the interpreter
    for(auto id : controlledObjects_) {
        UpdateObject(GetObject(id));
    }

    UpdateObject(GetObject(0));

    auto& ego = GetObject(0);
    if (ego.x <= 0) {
        // set variable 2 to 4
    }
}

namespace {

inline bool ObjectAtPosition(const Object& object, uint8_t x, uint8_t y)
{
    return (object.x == x) && (object.y == y);
}

uint8_t GetObjectPriority(Object& object)
{
    if (object.autoPriority) {
        static const uint8_t levels[] = {48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168 };
        for(size_t i = 0; i < (sizeof(levels) / sizeof(uint8_t)); ++i) {
            if (object.y < levels[i]) {
                return i + 4;
            }
        }
    }
    return object.priority;
}

} // namespace

void ObjectTable::UpdateObject(Object& object)
{
    switch(object.motion) {
    case Motion::kNormal:
        NormalMotion(object);
        break;
    case Motion::kMoveObject:
        MoveObjectToPosition(object);
        break;
    default:
        break;
    }
}

void ObjectTable::NormalMotion(Object& object)
{
    object.stepSize = 1;
    int dx, dy;
    switch(object.direction) {
    case Direction::kStationary:
        dx = dy = 0;
        break;
    case Direction::kNorth:
        dx = 0;
        dy = -object.stepSize;
        break;
    case Direction::kNorthEast:
        dx = object.stepSize;
        dy = -object.stepSize;
        break;
    case Direction::kEast:
        dx = object.stepSize;
        dy = 0;
        break;
    case Direction::kSouthEast:
        dx = object.stepSize;
        dy = object.stepSize;
        break;
    case Direction::kSouth:
        dx = 0;
        dy = object.stepSize;
        break;
    case Direction::kSouthWest:
        dx = -object.stepSize;
        dy = object.stepSize;
        break;
    case Direction::kWest:
        dx = -1; // object.stepSize;
        dy = 0;
        break;
    case Direction::kNorthWest:
        dx = -object.stepSize;
        dy = -object.stepSize;
        break;
    }
    object.x += dx;
    object.y += dy;
}

void ObjectTable::MoveObjectToPosition(Object& object)
{
    if (ObjectAtPosition(object, object.dstX, object.dstY)) {
        // object is at the desired position
        object.motion = Motion::kNormal;
        // set the flag to indicate that movement is complete
        flags_.set(object.moveFlagToSet);
    }
    else {
        float dx = static_cast<float>(object.dstX) - static_cast<float>(object.x);
        float dy = static_cast<float>(object.dstY) - static_cast<float>(object.y);
        float distance = sqrt(dx*dx + dy*dy);

        if (distance < object.stepSize) {
            // move to the object in a single step
            object.x = object.dstX;
            object.y = object.dstY;
        }
        else {
            object.x += dx / distance;
            object.y += dy / distance;
        }
    }
}

void ObjectTable::MoveObject(
    uint8_t objectNumber, uint8_t x, uint8_t y, uint8_t stepSize, uint8_t flag)
{
    auto& object = GetObject(objectNumber);
    object.motion = Motion::kMoveObject;
    object.stepSize = stepSize;
    object.moveFlagToSet = flag;
    object.dstX = x;
    object.dstY = y;
}

/*****************************************************************************/
/*                                  Drawing                                  */
/*****************************************************************************/
void ObjectTable::DrawObjects(Framebuffer& framebuffer)
{
    // for each displayed object
    for(auto id : displayedObjects_) {
        DrawObject(GetObject(id), framebuffer);
    }

    for(auto id : controlledObjects_) {
        AnimationTick(GetObject(id));
    }

    auto& ego = GetObject(0);

    ego.cycling = GetDirection(0) == Direction::kStationary ? 0 : 1;


    AnimationTick(GetObject(0));
}

void ObjectTable::DrawObject(Object& object, Framebuffer& framebuffer)
{
    if (!object.viewInstance) {
        // skip the object if it does not have an associated view instance
        assert(false);
        return;
    }
    if (object.loop >= object.viewInstance->loops.size()) {
        // invalid loop index, ignore
        //assert(false);
        return;
    }
    // get the loop
    auto& loop = object.viewInstance->loops.at(object.loop);
    if (object.cel >= loop.cels.size()) {
        //assert(false);
        // invalid cel, ignore
        return;
    }

    // now paint the Cel
    PaintCel(framebuffer, loop.cels.at(object.cel), object.x, object.y, GetObjectPriority(object), object.loop);
}

uint8_t ObjectTable::GetBaselineWidth(Object& object)
{
    if (!object.viewInstance) {
        return 0;
    }
    if (object.loop >= object.viewInstance->loops.size()) {
        return 0;
    }
    // get the loop
    auto& loop = object.viewInstance->loops.at(object.loop);
    if (object.cel >= loop.cels.size()) {
        return 0;
    }
    return loop.cels.at(object.cel).width;
}

void ObjectTable::AnimationTick(Object& object)
{
    if (!object.cycling) {
        // not cycling, ignore!
        return;
    }

    if (!object.viewInstance) {
        return;
    }

    switch(object.cycleType) {
    case AnimationCycle::kNormal:
        object.cel = (object.cel + 1) % object.numberOfCels;
        break;
    case AnimationCycle::kEndOfLoop:
        if ((object.cel + 1) >= object.numberOfCels) {
            object.cycling = 0;
            flags_.set(object.animFlagToSet);
        }
        else {
            ++object.cel;
        }
        break;
    case AnimationCycle::kReverseLoop:
        if (object.cel == 0) {
            object.cycling = 0;
            flags_.set(object.animFlagToSet);
        }
        else {
            --object.cel;
        }
        break;
    case AnimationCycle::kReverseCycle:
        if (object.cel == 0) {
            object.cycling = 0;
        }
        else {
            --object.cel;
        }
        break;
    default:
        break;
    }
}

void ObjectTable::PaintCel(
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
            else {
                //framebuffer.SetPixelIfHigherPriority(x + col, dstY, 4, priority);   
            }
        }
    }
}

void ObjectTable::AnimateObj(uint8_t objectNumber)
{
    controlledObjects_.insert(objectNumber);
    auto& obj = GetObject(objectNumber);
    obj.cycling = 1;   
}

void ObjectTable::UnanimateAll()
{
    controlledObjects_.clear();
}

void ObjectTable::SetView(uint8_t objectNumber, uint8_t viewNumber)
{
    auto& object = GetObject(objectNumber);
    object.view = viewNumber;
    object.viewInstance = views_.GetView(viewNumber);
    object.cel = 0;
    if (object.viewInstance) {
        auto& loops = object.viewInstance->loops;
        object.numberOfLoops = loops.size();
        if (object.numberOfLoops) {
            auto& loop = loops.front();
            object.numberOfCels = loop.cels.size();
        }
        else {
            object.numberOfCels = 0;
        }
    }
}

void ObjectTable::SetLoop(uint8_t objectNumber, uint8_t loopNumber)
{
    auto& object = GetObject(objectNumber);
    object.loop = loopNumber;
    if (object.viewInstance) {
        auto& loops = object.viewInstance->loops;
        if (loopNumber < loops.size()) {
            object.numberOfCels = loops.at(loopNumber).cels.size(); 
        }
    }
}

void ObjectTable::FixLoop(uint8_t objectNumber)
{
    auto& object = GetObject(objectNumber);
    object.loopFixed = 1;
}

void ObjectTable::ReleaseLoop(uint8_t objectNumber)
{
    auto& object = GetObject(objectNumber);
    object.loopFixed = 0;
}

void ObjectTable::Draw(uint8_t objectNumber)
{
    displayedObjects_.insert(objectNumber);
}

void ObjectTable::Erase(uint8_t objectNumber)
{
    displayedObjects_.erase(objectNumber);
}

void ObjectTable::SetCel(uint8_t objectNumber, uint8_t cel)
{
    auto& object = GetObject(objectNumber);
    object.cel = cel;   
}

void ObjectTable::GetPosition(uint8_t objectNumber, uint8_t& x, uint8_t& y)
{
    auto& object = GetObject(objectNumber);
    x = object.x;
    y = object.y;
}

void ObjectTable::Reposition(uint8_t objectNumber, uint8_t x, uint8_t y)
{

}

void ObjectTable::SetPosition(uint8_t objectNumber, uint8_t x, uint8_t y)
{
    auto& object = GetObject(objectNumber);
    object.x = x;
    object.y = y;
}

void ObjectTable::LastCel(uint8_t objectNumber, uint8_t&)
{

}

void ObjectTable::CurrentCel(uint8_t objectNumber, uint8_t& result)
{
    auto& object = GetObject(objectNumber);
    result = object.cel;
}

void ObjectTable::CurrentLoop(uint8_t objectNumber, uint8_t& result)
{
    auto& object = GetObject(objectNumber);
    result = object.loop;
}

void ObjectTable::CurrentView(uint8_t objectNumber, uint8_t& result)
{
    auto& object = GetObject(objectNumber);
    result = object.view;
}

void ObjectTable::GetNumberOfLoops(uint8_t objectNumber, uint8_t& result)
{
    auto& object = GetObject(objectNumber);
    result = object.numberOfLoops;
}

void ObjectTable::SetPriority(uint8_t objectNumber, uint8_t priority)
{
    auto& object = GetObject(objectNumber);
    object.priority = priority;
    object.autoPriority = 0;
}

void ObjectTable::AutoPriority(uint8_t objectNumber)
{
    auto& object = GetObject(objectNumber);
    object.autoPriority = 1;
}

void ObjectTable::GetPriority(uint8_t objectNumber, uint8_t& result)
{
    auto& object = GetObject(objectNumber);
    if (object.autoPriority) {
        static const uint8_t levels[] = {48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168 };
        for(size_t i = 0; i < (sizeof(levels) / sizeof(uint8_t)); ++i) {
            if (object.y < levels[i]) {
                result = i + 4;
                return;
            }
        }
    }
    else {
        result = object.priority;
    }
}

void ObjectTable::ObserveHorizon(uint8_t objectNumber, bool observeHorizon)
{
    auto& object = GetObject(objectNumber);
    object.observeHorizon = observeHorizon ? 1 : 0;
}

void ObjectTable::StartUpdate(uint8_t objectNumber)
{
    controlledObjects_.insert(objectNumber);
}

void ObjectTable::StopUpdate(uint8_t objectNumber)
{
    controlledObjects_.erase(objectNumber);
}

void ObjectTable::ForceUpdate(uint8_t objectNumber)
{

}

void ObjectTable::StartCycling(uint8_t objectNumber)
{
    auto& object = GetObject(objectNumber);
    object.cycling = 1;
}

void ObjectTable::StopCycling(uint8_t objectNumber)
{
    auto& object = GetObject(objectNumber);
    object.cycling = 0;
}

void ObjectTable::NormalCycle(uint8_t objectNumber)
{
    auto& object = GetObject(objectNumber);
    object.cycleType = AnimationCycle::kNormal;
}

void ObjectTable::ReverseCycle(uint8_t objectNumber)
{

}

void ObjectTable::EndOfLoop(uint8_t objectNumber, uint8_t flag)
{
    auto& object = GetObject(objectNumber);
    object.animFlagToSet = flag;
    object.cycleType = AnimationCycle::kEndOfLoop;
    object.cycling = 1;
}

void ObjectTable::ReverseLoop(uint8_t objectNumber, uint8_t flag)
{
    auto& object = GetObject(objectNumber);
    object.animFlagToSet = flag;
    object.cycleType = AnimationCycle::kReverseLoop;
    object.cycling = 1;
}

void ObjectTable::SetCycleTime(uint8_t objectNumber, uint8_t time)
{

}

void ObjectTable::IgnoreObjects(uint8_t objectNumber, bool ignore)
{
    auto& object = GetObject(objectNumber);
    object.observeObjects = !ignore;
}

void ObjectTable::SetHorizon(uint8_t horizon)
{

}

void ObjectTable::SetAllowedSurface(uint8_t objectNumber, SurfaceType type)
{

}

uint8_t ObjectTable::Distance(uint8_t object1, uint8_t object2)
{
    return 255;
}

void ObjectTable::StopMotion(uint8_t objectNumber)
{

}

void ObjectTable::StartMotion(uint8_t objectNumber)
{

}

void ObjectTable::SetStepSize(uint8_t objectNumber, uint8_t stepSize)
{

}

void ObjectTable::SetStepTime(uint8_t objectNumber, uint8_t stepTime)
{
    
}

void ObjectTable::SetDirection(uint8_t objectNumber, Direction direction)
{
    auto& object = GetObject(objectNumber);
    object.direction = direction;
}

Direction ObjectTable::GetDirection(uint8_t objectNumber)
{
    auto& object = GetObject(objectNumber);
    return object.direction;
}

bool ObjectTable::ObjectInBox(
    uint8_t obj, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    auto& object = GetObject(obj);
    auto width = GetBaselineWidth(object);
    if (!width) {
        return false;
    }

    if ((object.y < y1) || (object.y > y2)) {
        return false;
    }

    if ((object.x > x2) || ((object.x + width) < x1)) {
        return false;
    }
    return true;
}

} // namespace agi
