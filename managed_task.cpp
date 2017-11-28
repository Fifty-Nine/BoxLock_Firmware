#include "managed_task.h"

#include "debug.h"

namespace {

enum task_flags : uint8_t {
    stoppedFlag =  0b01,
    startedFlag = 0b10,
};

}

tasks::managed_task::managed_task(
    const char *name,
    StackType_t *stack,
    size_t stackSize,
    event_msg *queueBuffer,
    size_t queueSize,
    UBaseType_t priority) :
    name { name },
    taskStack { stack },
    taskStackSize { stackSize },
    priority { priority },
    queueHandle {
        xQueueCreateStatic(
            queueSize,
            sizeof(event_msg),
            reinterpret_cast<uint8_t*>(queueBuffer),
            &queue
        )
    },
    flagsHandle { xEventGroupCreateStatic(&flags) }
{
    auto rc = xTaskCreateStatic(
        [](void* c) { ((managed_task*)c)->run(); },
        name,
        stackSize,
        this,
        priority,
        &taskHandle,
        stack,
        &task
    );
    debug::assert(rc != pdFALSE);
    debug::assert(queueHandle);
    debug::assert(flagsHandle);
}

bool tasks::managed_task::post(
    const event_msg& msg, TickType_t timeout)
{
    return xQueueSendToBack(queueHandle, &msg, timeout) == pdTRUE;
}

bool tasks::managed_task::postFromISR(const event_msg& msg)
{
    BaseType_t woken;
    return xQueueSendToBackFromISR(queueHandle, &msg, &woken) == pdTRUE;
}

TaskHandle_t tasks::managed_task::handle() const
{
    return taskHandle;
}

bool tasks::managed_task::running() const
{
    auto bits = xEventGroupGetBitsFromISR(flagsHandle);
    return (bits & startedFlag) && !(bits & stoppedFlag);
}

bool tasks::managed_task::join(TickType_t timeout)
{
    if (timeout == 0) {
        return (xEventGroupGetBitsFromISR(flagsHandle) & stoppedFlag) != 0;
    }
    return (xEventGroupWaitBits(
        flagsHandle,
        stoppedFlag,
        pdFALSE,
        pdTRUE,
        timeout
    ) & stoppedFlag) == stoppedFlag;
}

void tasks::managed_task::restart()
{
    debug::assert(join(0) == true);
    xQueueReset(queueHandle);
    xEventGroupClearBits(flagsHandle, startedFlag | stoppedFlag);

    auto rc = xTaskCreateStatic(
        [](void* c) { ((managed_task*)c)->run(); },
        name,
        taskStackSize,
        this,
        priority,
        &taskHandle,
        taskStack,
        &task
    );
    debug::assert(rc != pdFALSE);
}

void tasks::managed_task::run()
{
    startup();
    xEventGroupSetBits(flagsHandle, startedFlag);
    while (true) {
        event_msg msg;
        if (xQueueReceive(queueHandle, (uint8_t*)&msg, portMAX_DELAY) == pdTRUE) {
            if (msg.cmd == event_msg::CMD_Stop) {
                break;
            } else if (msg.cmd >= event_msg::CMD_User) {
                process(msg);
            }
        }
    }

    teardown();
    xEventGroupSetBits(flagsHandle, stoppedFlag);
    xEventGroupClearBits(flagsHandle, startedFlag);
    vTaskDelete(taskHandle);
}
