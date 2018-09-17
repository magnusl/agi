#pragma once

#include <agi/view_loader.h>
#include <agi/framebuffer.h>
#include <agi/object.h>
#include <stdint.h>
#include <set>
#include <array>
#include <bitset>

namespace agi {



/**
 * \class   ObjectTable
 */
class ObjectTable
{
public:
    ObjectTable(ViewLoader&, std::bitset<256>& flags);

    void SetDirection(uint8_t objectNumber, Direction);
    Direction GetDirection(uint8_t objectNumber);

    void UpdateObjects();
    void DrawObjects(Framebuffer&);
    void AnimateObj(uint8_t objectNumber);
    void UnanimateAll();
    void SetView(uint8_t objectNumber, uint8_t viewNumber);
    void SetLoop(uint8_t objectNumber, uint8_t loopNumber);
    void FixLoop(uint8_t objectNumber);
    void ReleaseLoop(uint8_t objectNumber);
    void Draw(uint8_t objectNumber);
    void Erase(uint8_t objectNumber);
    void SetCel(uint8_t objectNumber, uint8_t);
    void GetPosition(uint8_t objectNumber, uint8_t& x, uint8_t& y);
    void Reposition(uint8_t objectNumber, uint8_t x, uint8_t y);
    void SetPosition(uint8_t objectNumber, uint8_t x, uint8_t y);
    void LastCel(uint8_t objectNumber, uint8_t&);
    void CurrentCel(uint8_t objectNumber, uint8_t&);
    void CurrentLoop(uint8_t objectNumber, uint8_t&);
    void CurrentView(uint8_t objectNumber, uint8_t&);
    void GetNumberOfLoops(uint8_t objectNumber, uint8_t&);

    void MoveObject(
        uint8_t objectNumber, uint8_t x, uint8_t y, uint8_t stepSize, uint8_t flag);

    void SetAllowedSurface(uint8_t objectNumber, SurfaceType type);
    void IgnoreObjects(uint8_t objectNumber, bool ignore);
    uint8_t Distance(uint8_t object1, uint8_t object2);

    void StopMotion(uint8_t objectNumber);
    void StartMotion(uint8_t objectNumber);
    void SetStepSize(uint8_t objectNumber, uint8_t stepSize);
    void SetStepTime(uint8_t objectNumber, uint8_t stepTime);

    /*************************************************************************/
    /*                                  Priority                             */
    /*************************************************************************/
    void SetPriority(uint8_t objectNumber, uint8_t priority);
    void AutoPriority(uint8_t objectNumber);
    void GetPriority(uint8_t objectNumber, uint8_t&);

    /*************************************************************************/
    /*                                  Updating                             */
    /*************************************************************************/
    void StartUpdate(uint8_t objectNumber);
    void StopUpdate(uint8_t objectNumber);
    void ForceUpdate(uint8_t objectNumber);

    /*************************************************************************/
    /*                                  Horizon                              */
    /*************************************************************************/
    void ObserveHorizon(uint8_t objectNumber, bool);
    void SetHorizon(uint8_t);

    /*************************************************************************/
    /*                                  Animation                            */
    /*************************************************************************/
    void StartCycling(uint8_t objectNumber);
    void StopCycling(uint8_t objectNumber);
    void NormalCycle(uint8_t objectNumber);
    void ReverseCycle(uint8_t objectNumber);
    void EndOfLoop(uint8_t objectNumber, uint8_t flag);
    void ReverseLoop(uint8_t objectNumber, uint8_t flag);
    void SetCycleTime(uint8_t objectNumber, uint8_t time);


    bool ObjectInBox(uint8_t obj, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);


private:
    Object& GetObject(uint8_t index) {
        return objects_[index];
    }

    void UpdateObject(Object& object);
    void DrawObject(Object& object, Framebuffer& framebuffer);
    void AnimationTick(Object& object);
    void MoveObjectToPosition(Object& object);
    void NormalMotion(Object& object);
    uint8_t GetBaselineWidth(Object& object);

    // paints a Cel
    void PaintCel(
        Framebuffer& framebuffer,
        const Cel& cel,
        size_t x,
        size_t y,
        uint8_t priority,
        size_t loopIndex);

private:
    ViewLoader& views_;
    std::bitset<256>& flags_;
    std::array<Object, 256> objects_;
    // The ID:s of the objects that are controlled by the interpreter
    std::set<uint8_t> controlledObjects_;
    std::set<uint8_t> displayedObjects_;

};

} // namespace agi
