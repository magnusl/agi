#include <agi/object.h>
#include <stdexcept>
#include <assert.h>
#include <iostream>

namespace agi {

void Animation::SetView(uint8_t index, std::shared_ptr<View>&& view)
{
    if (!view) {
        throw std::invalid_argument("Invalid view instance.");
    }
    viewInstance = view;
    viewIndex = index;
    // set the number of loops
    numberOfLoops = view->loops.size();
    // default to the first loop
    SetLoop(0);
}

void Animation::SetLoop(uint8_t index)
{
    loopIndex = index;
    if (viewInstance) {
        numberOfCels = viewInstance->loops.at(loopIndex).cels.size();
    }
}

void Animation::SetCel(uint8_t index)
{
    celIndex = index;
}

void Animation::StartCycling()
{

    //cycling = true;
}

void Animation::StopCycling()
{
    //cycling = false;
}

void Animation::NormalCycle()
{
    cycleType = AnimationCycle::kNormal;
}

void Animation::EndOfLoop(uint8_t flagToSet)
{
    cycleType = AnimationCycle::kEndOfLoop;
    flag = flagToSet;
    //cycling = true;
}

void Animation::ReverseCycle()
{
    cycleType = AnimationCycle::kReverseCycle;
}

void Animation::ReverseLoop(uint8_t flagToSet)
{
    cycleType = AnimationCycle::kReverseLoop;
    flag = flagToSet;
    //cycling = true;
}

uint8_t Animation::LastCel() const
{
    return numberOfCels ? numberOfCels - 1 : 0;
}

} // namespace agi
