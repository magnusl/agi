#include <agi/commands.h>
#include <iostream>

namespace agi {

namespace {

const char* cmds[] = {
    "return",
    "increment",
    "decrement",
    "assignn",
    "assignv", // 0x04
    "addn",
    "addv",
    "subn",
    "subv",
    "lindirectv",  // 0x09
    "rindirect",
    "lindirectn",
    "set",
    "reset",
    "toggle",
    "set.v",
    "reset.v",
    "toggle.v",
    "new.room",
    "new.room.v",
    "load.logics",
    "load.logics.v", // 0x15
    "call",
    "call.v",
    "load.pic",
    "draw.pic",
    "show.pic",
    "discard.pic",
    "overlay.pic",
    "show.pri.screen",
    "load.view",
    "load.view.v",
    "discard.view",
    "animate.obj",
    "unanimate.all",
    "draw",             // 0x23
    "erase",
    "position",
    "position.v",
    "get.posn",
    "reposition",
    "set.view",
    "set.view.n",
    "set.loop",
    "set.loop.v",
    "fix.loop",
    "release.loop",
    "sel.cel",
    "sel.cel.v",
    "last.cel",
    "current.cel",
    "current.loop",
    "current.view",     // 0x34
    "number.of.loops",
    "set.priority",
    "set.priority.v",
    "release.priority",
    "get.priority",
    "stop.update",
    "start.update,",
    "force.update",
    "ignore horizon",
    "observe.horizon",
    "set.horizon",
    "object.on.water",
    "object.on.land",
    "object.on.anything",
    "ignore.objs",
    "observe.objs",
    "distance",         // 0x45
    "stop.cycling",
    "start.cycling",
    "normal.cycle",
    "end.of.loop",
    "reverse.cycle",
    "reverse.loop",
    "cycle.time",
    "stop.motion",
    "start.motion",
    "step.size",
    "step.time",
    "move.obj",
    "move.obj.v.",     // 0x52
    "follow.ego",
    "wander",
    "normal.motion",
    "set.dir",
    "get.dir",
    "ignore.blocks",
    "observe.blocks",
    "block",
    "unblock",
    "get",
    "get.v",
    "drop",
    "put",
    "put.v",
    "get.room.v",
    "load.sound",
    "sound",            // 0x63
    "stop.sound",
    "print",
    "print.v",
    "display",
    "display.v",
    "clear.lines",
    "text.screen",
    "graphics",
    "set.cursor.char",
    "set.text.attribute",
    "shake.screen",
    "configure.screen",
    "status.line.on",
    "status.line.off",
    "set.string",
    "get.string",
    "word.to.string",
    "parse",
    "get.num",
    "prevent.input",
    "accept.input",
    "set.key",
    "add.to.pic",
    "add.to.pic.v",
    "status",
    "save.game",
    "restore.game",
    "init.disk",
    "restart.game",
    "show.obj",
    "random",
    "program.control",
    "player.control",
    "obj.status.v",
    "quit",                 // 0x86
    "show.mem",
    "pause",
    "echo.line",
    "cancel.line",
    "init.joy",
    "toggle.monitor",
    "version",
    "script.size",
    "set.game.id",
    "log",
    "set.scan.start",
    "reset.scan.start",
    "reposition.to",
    "reposition.to.v",
    "trace.on",
    "trace.info",
    "print.at",
    "print.at.v",
    "discard.view.v",
    "clear.text.rect",
    "set.upper.left",
    "set.menu",
    "set.menu.item",
    "submit.menu",
    "enable.item",
    "disable.item",
    "menu.input",
    "show.obj.v",
    "open.dialogue",
    "close.dialogue",
    "mul.n",
    "mul.v",
    "div.n",
    "div.v",
    "close.window"
};

} // namespace

const char* GetCommandName(uint8_t cmd)
{
    if (cmd < (sizeof(cmds)/sizeof(cmds[0]))) {
        return cmds[cmd];
    }
    else if (cmd == 0xff) {
        return "if";
    }
    else if (cmd == 0xfe) {
        return "else/goto";
    }
    else {
        return "unknown";
    }
}

} // namespace agi
