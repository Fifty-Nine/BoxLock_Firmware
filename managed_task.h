#ifndef MANAGED_TASK_H
#define MANAGED_TASK_H

#include <FreeRTOS.h>
#include <event_groups.h>
#include <queue.h>
#include <task.h>

namespace tasks {

class managed_task
{
public:
    struct event_msg
    {
        enum cmd_t : uint8_t {
            CMD_Stop,
            CMD_User
        };
        uint8_t cmd;
        uint8_t data[15];
    } __attribute__((packed));
    static_assert(sizeof(event_msg) == 16);

    managed_task(
        const char *name,
        StackType_t *stack,
        size_t stackSize,
        event_msg *queueBuffer,
        size_t queueSize,
        UBaseType_t priority = tskIDLE_PRIORITY + 1);

    bool running() const;
    bool post(const event_msg& msg, TickType_t timeout = portMAX_DELAY);
    bool postFromISR(const event_msg& msg);
    void stop();
    void restart();

    TaskHandle_t handle() const;
    bool join(TickType_t timeout = portMAX_DELAY);

private:
    virtual void process(event_msg& msg) = 0;
    virtual void startup() { }
    virtual void teardown() { }
    void run();

    const char *name;
    StackType_t *taskStack;
    size_t taskStackSize;
    UBaseType_t priority;

    StaticTask_t task;
    StaticQueue_t queue;
    StaticEventGroup_t flags;

    TaskHandle_t taskHandle;
    QueueHandle_t queueHandle;
    EventGroupHandle_t flagsHandle;
};

} /* namespace tasks */
#endif /* MANAGED_TASK_H */
