#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <malloc.h>

#include "app_tasks.h"
#include "console.h"
#include "box_control.h"
#include "linenoise.h"
#include "rtos_port.h"
#include "mcu.h"
#include "timers.h"

namespace {

struct command_t
{
	const char *name;
	void (* const callback)(const char *);
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
	
	
	if (trySetPin(oldPin, newPin)) {
		printf("New PIN successfully set.\n");
	} else {
		printf("Invalid PIN.\n");
	}
}

extern "C" void* sbrk(intptr_t);

void memoryStats(const char*)
{
    auto info = mallinfo();
    printf(
        "Heap:\n"
        "\t- heap size     = %#8x\n"
        "\t- in use blocks = %#8x\n"
        "\t- free blocks   = %#8x\n"
        "\t- current sbrk  = %p\n",
        info.arena,
        info.uordblks,
        info.fordblks,
        sbrk(0)
    );

    printf(
        "Task stacks:\n"
        "\t- Keypad  = %#8lx\n"
        "\t- Lock    = %#8lx\n"
        "\t- Console = %#8lx\n"
        "\t- Idle    = %#8lx\n"
        "\t- Timer   = %#8lx\n",
        uxTaskGetStackHighWaterMark(tasks::keypadScan),
        uxTaskGetStackHighWaterMark(tasks::lockControl),
        uxTaskGetStackHighWaterMark(tasks::console),
        uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandle()),
        uxTaskGetStackHighWaterMark(xTimerGetTimerDaemonTaskHandle())
    );
}

command_t commands[] = {
	{ "clear", [](const char *) { linenoiseClearScreen(); }, "\t\tClear the screen.", nullptr },
	{
		"unlock", 
		[](const char *pin) {
			if (tryUnlock(pin)) {
				printf("Box unlocked.\n");
			} else {
				printf("Invalid pin.\n");
			}
		},
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
        [](const char*) { mcu::reset(); },
        "\tReset the MCU.",
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

} /* namespace */

TaskHandle_t tasks::console = NULL;
void console::startTask()
{
    if (!tasks::console) {
        xTaskCreate(
            &consoleTask,
            "Command Line",
            2048,
            NULL,
            tskIDLE_PRIORITY+1,
            &tasks::console
        );
    }
}

void console::stopTask()
{
    if (tasks::console) {
        vTaskDelete(tasks::console);
        tasks::console = NULL;
    }
}
