#if !defined(__input_task_h__)
#define __input_task_h__
#include <Arduino.h>
#include <TaskScheduler.h>

Scheduler input_scheduler;
// Callback methods prototypes
void input_task_watcher_handle_function();

class Input : public Task
{
  typedef void (*ChangeCallback)(Input &);

public:
  enum InputStatus
  {
    KEYDOWN = 0,
    KEYUP = 1,
    HOLD = 2
  };

private:
  byte ref;
  InputStatus status;
  ChangeCallback onChange;

  void init(byte _pin, byte mode, ChangeCallback cb = NULL)
  {
    pin = _pin;
    pinMode(_pin, mode);
    value = digitalRead(_pin);
    time = millis();
    status = (value == ref ? KEYDOWN : KEYUP);
    this->Task::setLtsPointer(this);
    onChange = cb;
  }

public:
  byte pin;
  byte value;
  uint32_t time;

  Input(byte _pin, byte mode = INPUT_PULLUP) : Task(50, TASK_FOREVER, &input_task_watcher_handle_function)
  {
    this->ref = LOW;
    this->init(_pin, mode);
  }

  Input(byte _pin, byte mode, ChangeCallback cb = NULL) : Task(50, TASK_FOREVER, &input_task_watcher_handle_function)
  {
    this->ref = LOW;
    this->init(_pin, mode, cb);
  }

  Input(byte _pin, byte mode, ChangeCallback cb, byte _ref) : Task(50, TASK_FOREVER, &input_task_watcher_handle_function)
  {
    this->ref = _ref;
    this->init(_pin, mode, cb);
  }

  InputStatus getStatus() { return status; }
  bool isDown() { return read() == ref; }
  bool isHold() { return isDown() && status == KEYDOWN && (millis() - this->time) >= 600; }
  byte read() { return digitalRead(this->pin); }
  void setRef(byte ref) { this->ref = ref; }

  void execute()
  {
    if (this->onChange != NULL)
      this->onChange(*this);
  }

  bool update()
  {
    if (this->isHold())
    {
      this->status = HOLD;
      this->time = millis();
      return true;
    }
    else if (this->read() == this->value)
      return false;

    else
    {
      this->time = millis();
      this->value = this->read();
      this->status = this->isDown() ? KEYDOWN : KEYUP;
      return true;
    }
  }
};

void input_task_watcher_handle_function()
{
  Task &task = input_scheduler.currentTask();
  Input *input = (Input *)task.getLtsPointer();
  if (input->update())
    input->execute();
}

void input_handle_multithreading_runner(void *ptr);

class __InputHandle
{
public:
  TaskHandle_t handle;

  __InputHandle()
  {
    input_scheduler.init();
  }

  void addInput(Input &input)
  {
    input_scheduler.addTask(input);
    input.enable();
  }

  void init()
  {

    xTaskCreate(
        input_handle_multithreading_runner,
        "Input scheduler runner", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
        3000,
        this,
        1,
        &handle);
  }
};

void input_handle_multithreading_runner(void *ptr)
{
  while (true)
  {
    input_scheduler.execute();
    Task *t = input_scheduler.getCurrentTask();

    if (t != NULL)
    {
      long time = input_scheduler.timeUntilNextIteration(*(t->getNextTask()));
      if (time > 0)
        vTaskDelay(time);
    }
  }
}

__InputHandle InputHandle;

#endif // __input_task_h__
