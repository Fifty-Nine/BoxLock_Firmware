#include "console.h"

#include <malloc.h>        // for mallinfo
#include <cctype>          // for isspace
#include <cstdio>          // for printf, NULL, size_t
#include <cstdint>         // for intptr_t
#include <cstdlib>         // for free
#include <cstring>         // for strcmp
#include <string>          // for string

#include "FreeRTOS.h"      // for StaticTask_t
#include "app_tasks.h"     // for console, keypadScan, lockControl
#include "debug.h"         // for debug::assert
#include "keypad.h"        // for keypad::beep
#include "linenoise.h"     // for linenoiseClearScreen, linenoise, linenoise...
#include "lock_control.h"  // for tryUnlock, trySetPin
#include "mcu.h"           // for reset
#include "portmacro.h"     // for StackType_t
#include "pwm.h"           // for pwm::enable/disable
#include "sleep.h"         // for sleep::enterSleep
#include "solenoid-params.h" // for solenoid::params
#include "task.h"          // for uxTaskGetStackHighWaterMark, vTaskDelete
#include "timers.h"        // for xTimerGetTimerDaemonTaskHandle

namespace {

using command_fn = void (*const)(char*);
struct command_t
{
    const char *name;
    command_fn callback;
    const char *documentation;
    const char *details;
    
    char *partialMatch(char *line) const
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
    
    char *match(char *line) const
    {
        auto partial = partialMatch(line);
        if (!partial || (*partial != '\0' && !std::isspace(*partial))) {
            return nullptr;
        }
        
        while (*partial != '\0' && std::isspace(*partial)) {
            ++partial;
        }
        return partial;
    }
};

unsigned splitArgs(char *buffer, char **args, unsigned max)
{
    char *c = buffer;
    unsigned rc = 0;
    while (1) {
        /* Skip leading whitespace. */
        while (*c != '\0' && std::isspace(*c)) { ++c; }

        char *begin = c;
        while (*c != '\0' && !std::isspace(*c)) { ++c; }

        if (c == begin) {
            return rc;
        }

        if (rc < max) {
            args[rc] = begin;

            if (*c != '\0') {
                *c++ = '\0';
            } else {
                return ++rc;
            }
        }
        rc++;
    }
}

void printHelp(const char *cmd);

void unlockCmd(char *args)
{
    unsigned count = splitArgs(args, &args, 1);
    if (count > 1) {
        printf("Too many arguments.\n");
        printHelp("unlock");
        return;
    } else if (count < 1) {
        printf("Too few arguments.\n");
        printHelp("unlock");
        return;
    }
    if (lock::tryUnlock(args)) {
        printf("Box unlocked.\n");
    } else {
        printf("Invalid pin.\n");
    }
}

void setPinCmd(char *args)
{
    char *argv[2];
    unsigned count = splitArgs(args, argv, 2);
    if (count < 1) {
        printf("Too few arguments.\n");
        printHelp("set-pin");
        return;
    } else if (count > 2) {
        printf("Too many arguments.\n");
        printHelp("set-pin");
        return;
    }
    if (lock::trySetPin(argv[0], argv[1])) {
        printf("New PIN successfully set.\n");
    } else {
        printf("Invalid PIN.\n");
    }
}

void paramCmd(char *args)
{
    static char *argv[2];
    unsigned argc = splitArgs(args, argv, 2);

    if (argc > 2) {
        printf("Too many arguments.\n");
        printHelp("param");
        return;
    }

    static auto p = solenoid::getParams();
    if (argc == 0) {
        printf("chargetime: %8d\n", p.charge_time);
        printf("drivetime:  %8d\n", p.drive_time);
        printf("holdtime:   %8d\n", p.hold_time);
        printf("pwmperiod:  %8d\n", p.pwm_period);
        printf("pwmduty:    %8d\n", p.pwm_duty);
        return;
    }

    unsigned *param =
        strcmp(argv[0], "chargetime") == 0 ? &p.charge_time :
        strcmp(argv[0], "drivetime") == 0  ? &p.drive_time :
        strcmp(argv[0], "holdtime") == 0   ? &p.hold_time :
        strcmp(argv[0], "pwmperiod") == 0  ? &p.pwm_period :
        strcmp(argv[0], "pwmduty") == 0    ? &p.pwm_duty :
                                             nullptr;

    if (!param) {
        printf("Invalid parameter name: %s\n", argv[0]);
        return;
    }

    if (argc == 1) {
        printf("%s: %4d\n", argv[0], *param);
        return;
    }

    char *endptr;
    auto new_val = strtoul(argv[1], &endptr, 0);

    if (*endptr == '\0') {
        *param = new_val;
        solenoid::setParams(p);
        printf("Value successfully updated.\n");
    } else {
        printf("Invalid integer: %s\n", argv[1]);
    }
}

void sleepCmd(char *arg)
{
    int argc = splitArgs(arg, &arg, 1);

    if (argc > 1) {
        printf("Too many arguments.\n");
        printHelp("sleep");
        return;
    }

    if (strcmp(arg, "on") == 0) {
        sleep::inhibit(false);
        printf("Sleep mode enabled.\n");
    } else if (strcmp(arg, "off") == 0) {
        sleep::inhibit(true);
        printf("Sleep mode disabled until next reset.\n");
    } else if (argc == 1) {
        printf("Unrecognized sleep command: %s\n", arg);
        printHelp("sleep");
    } else {
        sleep::enterSleep();
    }
}

extern "C" void* sbrk(intptr_t);
extern "C" intptr_t __heap_max__;

void memoryStats(char*)
{
    static auto info = mallinfo();

    char* curr_sbrk = (char*)sbrk(0);
    ptrdiff_t unused = (char*)&__heap_max__ - curr_sbrk;

    ptrdiff_t unused_kb = (unused * 10) >> 10;
    ptrdiff_t unused_tenth_kb = unused_kb % 10;
    unused_kb /= 10;

    printf(
        "Heap:\n"
        "\t- heap size     = %#10x\n"
        "\t- in use blocks = %#10x\n"
        "\t- free blocks   = %#10x\n"
        "\t- current sbrk  = %10p\n"
        "\t- sbrk max      = %10p\n"
        "\t- unused memory = %7d.%dK\n\n",
        info.arena,
        info.uordblks,
        info.fordblks,
        curr_sbrk,
        &__heap_max__,
        unused_kb,
        unused_tenth_kb
    );

    printf("Task stacks:\n");

    auto hwm = uxTaskGetStackHighWaterMark(tasks::keypadScan);
    printf("\t- Keypad (Scan)    = %#8lx %8ld\n", hwm, hwm);
    hwm = uxTaskGetStackHighWaterMark(tasks::keypadControl);
    printf("\t- Keypad (Control) = %#8lx %8ld\n", hwm, hwm);
    hwm = uxTaskGetStackHighWaterMark(tasks::lockControl);
    printf("\t- Lock             = %#8lx %8ld\n", hwm, hwm);
    hwm = uxTaskGetStackHighWaterMark(tasks::console);
    printf("\t- Console          = %#8lx %8ld\n", hwm, hwm);
    hwm = uxTaskGetStackHighWaterMark(tasks::usb);
    printf("\t- USB              = %#8lx %8ld\n", hwm, hwm);
    hwm = uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandle());
    printf("\t- Idle             = %#8lx %8ld\n", hwm, hwm);
    hwm = uxTaskGetStackHighWaterMark(xTimerGetTimerDaemonTaskHandle());
    printf("\t- Timer            = %#8lx %8ld\n\n", hwm, hwm);
}

void pwmCmd(char *arg)
{
    int argc = splitArgs(arg, &arg, 1);
    if (argc > 1) {
        printf("Too many arguments.");
        printHelp("pwm");
        return;
    }

    if (strcmp(arg, "on") == 0) {
        printf("PWM output enabled.\n");
        auto params = solenoid::getParams();
        pwm::enable(params.pwm_period, params.pwm_duty);
    } else if (strcmp(arg, "off") == 0) {
        printf("PWM output disabled.\n");
        pwm::disable();
    } else {
        printf("Unrecognized PWM command: %s\n", arg);
        printHelp("pwm");
    }
}

void beepCmd(char *args)
{
    int argc = splitArgs(args, &args, 1);
    if (argc > 1) {
        printf("Too many arguments.");
        printHelp("beep");
        return;
    }

    char *endptr;
    unsigned len = argc == 1 ?
        strtoul(args, &endptr, 0) :
        200;

    if (argc ==1 && *endptr != '\0') {
        printf("Invalid integer: %s\n", args);
        return;
    }

    keypad::beep(len);
}

command_t commands[] __attribute__((section(".rodata#"))) = {
    {
        "clear",
        (command_fn)&linenoiseClearScreen,
        "\t\tClear the screen.",
        "Usage: clear\n"
        "Clears the screen.\n"
    },
    {
        "unlock", 
        &unlockCmd,
        "\tTry to unlock the box with the provided PIN.",
        "Usage: unlock PIN\n"
        "Try to unlock the box with the provided PIN.\n"
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
        "param",
        &paramCmd,
        "\t\tShow or update various low-level configuration parameters.",
        "Usage: param [PARAM] [VALUE]\n"
        "Show or set the VALUE of PARAM. If no arguments are specified,\n"
        "display the value of all valid parameters.\n"
    },
    {
       "reset",
       (command_fn)mcu::reset,
        "\t\tReset the MCU.",
        "Usage: reset\n"
        "Reset the MCU.\n"
    },
    {
        "sleep",
        &sleepCmd,
        "\t\tPut the MCU to sleep or enable or disable sleep mode.",
        "Usage: sleep [on|off]\n"
        "With \"on\" or \"off\", enable or disable sleep mode. Otherwise,\n"
        "immedately put the MCU to sleep. This will end the terminal session.\n"
    },
    {
        "pwm",
        (command_fn)&pwmCmd,
        "\t\tManually enable or disable the CPU_PWM output.",
        "Usage: pwm (on|off)\n"
        "Manually enable or disable the CPU_PWM output.\n",
    },
    {
        "beep",
        (command_fn)&beepCmd,
        "\t\tEmit an audible beep.",
        "Usage: beep [LENGTH]\n"
        "Emit an audible beep for LENGTH milliseconds. If unspecified,\n"
        "LENGTH defaults to 200 ms.\n"
    },
    {
        "mem-stats",
        &memoryStats,
        "\tPrint statistics about current memory usage.",
        "Usage: mem-stats\n"
        "Print statistics about current memory usage.\n"
    },
    {
        "help",
        (command_fn)&printHelp,
        "\t\tPrint this help screen. Additional information about a command\n"
        "\t\tmay be available with 'help [command]'.",
        "Usage: help [COMMAND]\n"
        "With no arguments, print the list of available commands. Otherwise,\n"
        "print the usage details of the given command.\n"
    },
};

static void completion(const char *line, linenoiseCompletions *lc)
{
    for (auto &cmd : commands) {
        if (cmd.partialMatch(const_cast<char*>(line))) {
            std::string name = cmd.name;
            name += ' ';
            linenoiseAddCompletion(lc, name.c_str());
        }
    }
}

command_t *findCommand(char *line, char **args = nullptr)
{
    for (auto &cmd : commands) {
        if (auto *rc = cmd.match(line)) {
            if (args) { *args = rc; }
            return &cmd;            
        }
    }
    return nullptr;
}

command_t *findCommand(const char *line)
{
    return findCommand(const_cast<char*>(line), nullptr);
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

extern "C" bool parseCommand(char* line_buffer)
{
    bool addToHistory = true;
    while (std::isspace(*line_buffer)) {
        addToHistory = false;
        ++line_buffer;
    }
    
    if (addToHistory) {
        linenoiseHistoryAdd(line_buffer);
    }

    char *args = nullptr;
    auto *cmd = findCommand(line_buffer, &args);
    
    if (cmd) {
        cmd->callback(args);
    }
    
    return cmd != NULL;
}

static void printBanner()
{
    linenoiseClearScreen();
    printf("Welcome to the LockBox v1.0 serial debug console!\n");
    printf("Type 'help' to see the available commands.\n");
}

void consoleTask(void*)
{
    linenoiseSetCompletionCallback(completion);
    printBanner();
    while (1) {
        char *c = linenoise("root@thebox:~> ");

        sleep::resetTimer();
        
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
