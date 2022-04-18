#ifndef _RTOS_CPP_H
#define _RTOS_CPP_H

#include <STM32FreeRTOS.h>
#include <USBSerial.h>
#include <string>
#include <functional>
#include <chrono>

extern void* pxCurrentTCB;
namespace quasi {

  template <class Lockable>
  class lock_guard {
  public:
    explicit lock_guard(Lockable& lockable, TickType_t wait = portMAX_DELAY) : lockable_(lockable) { lockable_.lock(wait); }
    inline ~lock_guard() { lockable_.unlock(); }
  private:

    Lockable& lockable_;

    lock_guard(lock_guard const&) = delete;      ///< We are not copyable.
    void operator =(lock_guard const&) = delete;  ///< We are not assignable.
  };

  class mutex {
  public:
	  mutex() {handle_ = xSemaphoreCreateMutex();}
	  inline ~mutex() { vSemaphoreDelete(handle_); }

	  inline bool lock(TickType_t wait = portMAX_DELAY) {
      BaseType_t success = xSemaphoreTake(handle_, wait);
      return success == pdTRUE ? true : false;
    }
	  inline bool unlock() {
      BaseType_t success = xSemaphoreGive(handle_);
      return success == pdTRUE ? true : false;
    }
  private:
    SemaphoreHandle_t handle_;

    mutex(mutex const&) = delete;      ///< We are not copyable.
    void operator =(mutex const&) = delete;  ///< We are not assignable.
  };

  class thread {
  public:
    using Callback = std::function<void()>;
    thread(const std::string& name, Callback&& cb, uint16_t stackDepth = 1024, 
      UBaseType_t priority = tskIDLE_PRIORITY) : callback_(std::move(cb)) {
      xTaskCreate(taskFunc, name.c_str(), stackDepth, this, priority, &handle_);
    }
    ~thread() {};

    inline void give_ISR() {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR( handle_, &xHigherPriorityTaskWoken );
      portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
    inline static uint32_t take(TickType_t waitTicks = portMAX_DELAY) {
      return ulTaskNotifyTake(pdTRUE, waitTicks);
    }

  private:
    static inline void taskFunc(void *pParams) {
      thread *th = static_cast<thread *>(pParams);
      th->callback_();
    } 

    Callback callback_;
    TaskHandle_t handle_;

    thread(thread const&) = delete;      ///< We are not copyable.
    void operator =(thread const&) = delete;  ///< We are not assignable.
};

  template<typename Data>
  class MsgQueue {  
  public:
  	explicit MsgQueue(unsigned queueLength) { handle_ = xQueueCreate(queueLength, sizeof(Data)); }    
    inline ~MsgQueue() { vQueueDelete(handle_);}

  	inline bool push(Data const& data, TickType_t time = 0) { return xQueueSend(handle_, &data, time) == pdPASS; }
	  inline bool pop(Data& data, TickType_t time = portMAX_DELAY) { return xQueueReceive(handle_, &data, time) == pdPASS; }
	  inline bool peek(Data& data, TickType_t time = 0) { return xQueuePeek(handle_, &data, time) == pdPASS; }

	  inline unsigned waiting() const { return uxQueueMessagesWaiting(handle_); }
	  inline unsigned available() const { return uxQueueSpacesAvailable(handle_); }
	  inline void reset() { xQueueReset(handle_); }

  private:
  
  	QueueHandle_t handle_;

    MsgQueue(MsgQueue const&) = delete;      ///< We are not copyable.
    void operator =(MsgQueue const&) = delete;  ///< We are not assignable.
  };

  namespace this_thread {
    
    inline uint32_t get_id() { return (uint32_t) pxCurrentTCB; }
//    inline pxCurrentTCB->uxTCBNumber
    inline void yield() noexcept { taskYIELD(); }
  }
}

#endif //_RTOS_CPP_H