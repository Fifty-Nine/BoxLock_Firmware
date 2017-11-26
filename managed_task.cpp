#include "managed_task.h"
#include "mcu.h"

tasks::managed_task::managed_task(
    const char *name,
    StackType_t *stack,
    size_t stackSize,
    event_msg *queueBuffer,
    size_t queueSize,
    UBaseType_t priority) :
    queueHandle {
        xQueueCreateStatic(
            queueSize,
            sizeof(event_msg),
            reinterpret_cast<uint8_t*>(queueBuffer),
            &queue
        )
    }
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
    mcu::assert(rc != pdFALSE);
    mcu::assert(queueHandle);
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

void tasks::managed_task::run()
{
    startup();
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
    vTaskDelete(taskHandle);
}
