#ifndef MANAGED_TASK_H
#define MANAGED_TASK_H

#include <FreeRTOS.h>
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

    bool post(const event_msg& msg, TickType_t timeout = portMAX_DELAY);
    bool postFromISR(const event_msg& msg);

    TaskHandle_t handle() const;

private:
    virtual void process(event_msg& msg) = 0;
    virtual void startup() { }
    virtual void teardown() { }
    void run();

    StaticTask_t task;
    StaticQueue_t queue;
    TaskHandle_t taskHandle;
    QueueHandle_t queueHandle;
};

} /* namespace tasks */
#endif /* MANAGED_TASK_H */
