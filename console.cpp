#include "console.h"

#include <malloc.h>        // for mallinfo
#include <cctype>          // for isspace
#include <cstdio>          // for printf, NULL, size_t
#include <cstdint>         // for intptr_t
#include <cstdlib>         // for free
#include <string>          // for string

#include "FreeRTOS.h"      // for StaticTask_t
#include "app_tasks.h"     // for console, keypadScan, lockControl
#include "debug.h"         // for debug::assert
#include "linenoise.h"     // for linenoiseClearScreen, linenoise, linenoise...
#include "lock_control.h"  // for tryUnlock, trySetPin
#include "mcu.h"           // for reset
#include "portmacro.h"     // for StackType_t
#include "task.h"          // for uxTaskGetStackHighWaterMark, vTaskDelete
#include "timers.h"        // for xTimerGetTimerDaemonTaskHandle

namespace {

using command_fn = void (*const)(const char*);
struct command_t
{
    const char *name;
    command_fn callback;
    const char *documentation;
    const char *details;
    
    const char *partialMatch(const char *line) const
    {
        size_t i = 0;
        for (; line[i] != '\0'; ++i) {
            char exp = name[i];
            if (exp == '\0') {
                
                if (!std::isspace(line[i])) {
                    return nullptr;
                }
                
                break;
            }
            if (line[i] != name[i]) {
                return nullptr;
            }
        }
        
        return line + i;
    }
    
    const char *match(const char *line) const
    {
        const char *partial = partialMatch(line);
        if (!partial || (*partial != '\0' && !std::isspace(*partial))) {
            return nullptr;
        }
        
        while (*partial != '\0' && std::isspace(*partial)) {
            ++partial;
        }
        return partial;
    }
};

void printHelp(const char *cmd);

void unlockCmd(const char *args)
{
    if (lock::tryUnlock(args)) {
        printf("Box unlocked.\n");
    } else {
        printf("Invalid pin.\n");
    }
}

void setPinCmd(const char *args)
{
    char oldPin[16];
    char *oldPinPtr = oldPin;
    while (*args != '\0' && !std::isspace(*args)) {
        *oldPinPtr = *args;
        ++oldPinPtr;
        ++args;
    }
    *oldPinPtr = '\0';
    while (*args != '\0' && std::isspace(*args)) { ++args; }
    const char *newPin = args;
    
    if (*oldPin == '\0' || *newPin == '\0') {
        printf("Missing argument.\n");
        printHelp("set-pin");
        return;
    }
    
    while (*args != '\0' && !std::isspace(*args)) { ++args; }
    while (*args != '\0' && std::isspace(*args)) {++args; }
    if (*args != '\0') {
        printf("Too many arguments.\n");
        printHelp("set-pin");
        return;
    }
    
    
    if (lock::trySetPin(oldPin, newPin)) {
        printf("New PIN successfully set.\n");
    } else {
        printf("Invalid PIN.\n");
    }
}

extern "C" void* sbrk(intptr_t);
extern "C" intptr_t __sram_end__;

void memoryStats(const char*)
{
    static auto info = mallinfo();

    char* curr_sbrk = (char*)sbrk(0);
    ptrdiff_t unused = (char*)&__sram_end__ - curr_sbrk;

    ptrdiff_t unused_kb = (unused * 10) >> 10;
    ptrdiff_t unused_tenth_kb = unused_kb % 10;
    unused_kb /= 10;

    printf(
        "Heap:\n"
        "\t- heap size     = %#10x\n"
        "\t- in use blocks = %#10x\n"
        "\t- free blocks   = %#10x\n"
        "\t- current sbrk  = %10p\n"
        "\t- unused memory = %7d.%dK\n\n",
        info.arena,
        info.uordblks,
        info.fordblks,
        curr_sbrk,
        unused_kb,
        unused_tenth_kb
    );

    printf(
        "Task stacks:\n"
        "\t- Keypad (Scan)    = %#8lx\n"
        "\t- Keypad (Control) = %#8lx\n"
        "\t- Lock             = %#8lx\n"
        "\t- Console          = %#8lx\n"
        "\t- USB              = %#8lx\n"
        "\t- Idle             = %#8lx\n"
        "\t- Timer            = %#8lx\n\n",
        uxTaskGetStackHighWaterMark(tasks::keypadScan),
        uxTaskGetStackHighWaterMark(tasks::keypadControl),
        uxTaskGetStackHighWaterMark(tasks::lockControl),
        uxTaskGetStackHighWaterMark(tasks::console),
        uxTaskGetStackHighWaterMark(tasks::usb),
        uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandle()),
        uxTaskGetStackHighWaterMark(xTimerGetTimerDaemonTaskHandle())
    );
}

command_t commands[] __attribute__((section(".rodata#"))) = {
    { "clear", (command_fn)&linenoiseClearScreen, "\t\tClear the screen.", nullptr },
    {
        "unlock", 
        &unlockCmd,
        "\tTry to unlock the box with the provided PIN.",
        nullptr
    },
    {
        "set-pin",
        &setPinCmd,
        "\tSet a new PIN number, given the old PIN and the new PIN.",
        "Usage: set-pin OLD NEW\n"
        "Set a new PIN number. If the given old pin is correct, it is discarded\n"
        "and the new PIN immediately becomes active. If the old PIN is incorrect,\n"
        "the new PIN is ignored and the console is temporarily locked.\n"
    },
    {
       "reset",
       (command_fn)mcu::reset,
        "\t\tReset the MCU.",
        nullptr
    },
    {
        "mem-stats",
        &memoryStats,
        "\tPrint statistics about current memory usage.",
        nullptr
    },
    {
        "help",
        &printHelp,
        "\t\tPrint this help screen. Additional information about a command\n"
        "\t\tmay be available with 'help [command]'.",
        nullptr
    }
};

static void completion(const char *line, linenoiseCompletions *lc)
{
    for (auto &cmd : commands) {
        if (cmd.partialMatch(line)) {
            std::string name = cmd.name;
            name += ' ';
            linenoiseAddCompletion(lc, name.c_str());
        }
    }
}

command_t *findCommand(const char *line, const char **args = nullptr)
{
    for (auto &cmd : commands) {
        if (auto *rc = cmd.match(line)) {
            if (args) { *args = rc; }
            return &cmd;            
        }
    }
    return nullptr;
}

void printHelp(const char *args)
{
    if (args && args[0] != '\0') {
        auto cmd = findCommand(args);
        if (cmd && (cmd->details || cmd->documentation)) {
            printf("%s\n", cmd->details ?: cmd->documentation);
        } else {
            printf("No help available for command [%s]\n", args);
        }
        return;
    }
    
    for (auto &cmd : commands) {
        if (cmd.documentation) {
            printf("[%s]%s\n", cmd.name, cmd.documentation);
        }
    }
}

extern "C" bool parseCommand(const char* line_buffer)
{
    bool addToHistory = true;
    while (std::isspace(*line_buffer)) {
        addToHistory = false;
        ++line_buffer;
    }

    const char *args = nullptr;
    auto *cmd = findCommand(line_buffer, &args);
    
    if (cmd) {
        cmd->callback(args);
    }
    
    if (addToHistory) {
        linenoiseHistoryAdd(line_buffer);
    }
    
    return cmd != NULL;
}

static void printBanner()
{
    linenoiseClearScreen();
    printf("Welcome to the LockBox v1.0 serial debug console!\n");
    printf("Type 'help' to see the available commands.");
    printf("\n");
}

void consoleTask(void*)
{
    linenoiseSetCompletionCallback(completion);
    printBanner();
    while (1) {
        char *c = linenoise("root@thebox:~> ");
        
        if (c && c[0] && !parseCommand(c)) {
            printf("Unknown command: %s\n", c);
        }
        
        free(c);
    }
}

StaticTask_t consoleTaskCtxt;
StackType_t consoleTaskStack[0xf0];

} /* namespace */

TaskHandle_t tasks::console = NULL;
void console::startTask()
{
    if (!tasks::console) {
        tasks::console = xTaskCreateStatic(
            &consoleTask,
            "Command Line",
            sizeof(consoleTaskStack) / sizeof(StackType_t),
            NULL,
            tskIDLE_PRIORITY+1,
            consoleTaskStack,
            &consoleTaskCtxt
        );
        debug::assert(tasks::console);
    }
}

void console::stopTask()
{
    if (tasks::console) {
        vTaskDelete(tasks::console);
        tasks::console = NULL;
    }
}
