/*                                                                                                                                                            
This file is a part of CronoS library
CronoS - Cron on RTOS, a lib that integrates RTOS tasks scheduling with cron semantics rules
                                                                                                                                                              
Copyright (C) Emil Muratov, 2024
GitHub: https://github.com/vortigont/CronoS
*/

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <list>
#include <memory>
#include <mutex>
#include <functional>
#include "ccronexpr.h"

using cronos_tid = uint32_t;

/**
 * @brief An abstract CronoS task
 * Implementation specific objects should derive from this class
 */
class CronoS_Task {
friend class CronoS;
  // task id
  cronos_tid _id{0};
protected:
  time_t next_run{};
  cron_expr rule{};
  bool valid;

public:
  explicit CronoS_Task(const char* expression);
  virtual ~CronoS_Task(){}

  // get task id
  cronos_tid getID() const { return _id; }

  const cron_expr& getExpr() const { return rule; }

  // set/update Task's cron expression
  void setExpr(const char* expr);

  /**
   * @brief a callback method that is triggered by a scheduler
   * 
   */
  virtual void cronos_run() = 0;

};

using CronoS_Task_pt = std::unique_ptr<CronoS_Task>;
// type for the CallBack function
using CronoS_Callback_t = std::function<void(cronos_tid id, void* arg)>;

/**
 * @brief CronoS task that implements functional callback
 * @note 
 * 
 */
class CronoS_Callback : public CronoS_Task {
protected:

  /**
   * @brief functional callback pointer
   * @param arg argumen object pointer
   */
  CronoS_Callback_t callback{nullptr};
  void* _arg;

public:
  CronoS_Callback(const char* expression, CronoS_Callback_t f, void* arg = nullptr) : CronoS_Task(expression), callback(f), _arg(arg) {}

  /*!
   * @copydoc CronoS_Task::cronos_run()
   * 
   */
  void cronos_run() override { if (callback) callback(getID(), _arg); }
};



class CronoS {
private:
  // mutex protects the access to tasks list container
  std::mutex _mtx;
  // counter to generate sequence num for task ids
  uint32_t _cnt{0};
  // a container that holds all scheduled tasks
  std::list< CronoS_Task_pt > _tasks;
  // RTOS timer to schedule task runs
  TimerHandle_t    _tmr{nullptr};

  void _evaluate();

public:
  ~CronoS();

  /**
   * @brief start scheduler runs
   * 
   */
  void start();

  /**
   * @brief stop scheduler runs
   * 
   */
  void stop();

  /**
   * @brief starts the scheduler and reevaluate all loaded rules
   * this method MUST be called in case of significant system date/time changes
   * to reevaluate loaded rules
   * 
   */
  void reload(){ start(); };

  /**
   * @brief clears all loaded tasks
   * 
   */
  void clear();

  /**
   * @brief create a new task based on `CronoS_Callback` object
   * it will execute provided functional callback based on scheduling rule 
   * 
   * @param expression crontab scheduling rule string 
   * @param cb functional callback to execute
   * @return cronos_tid is a Task ID that identifies the task in the scheduler 
   */
  cronos_tid addCallback(const char* expression, CronoS_Callback_t cb, void* arg = nullptr);

  /**
   * @brief remore a Task from a scheuler identifid by id
   * if no such task exists then this call does nothing
   * 
   * @param id CronoS task id
   */
  void removeTask(cronos_tid id);

  /**
   * @brief Get Crontab string for a task
   * 
   * @param id task id
   * @param buffer char buffer to write to (be sure to reserve enough)
   * @param buffer_len buffer size
   * @param expr_len number of cron fields produced
   * @param error output error message, will be set to string literal
   * @return int used length of the buffer or -1 on error
   */
  int getCrontab(cronos_tid id, char *buffer, int buffer_len, int expr_len = 6, const char **error = NULL) const;

  /**
   * @brief Set/update cron expression for task with id
   * 
   * @param id 
   * @param expr 
   */
  void setExpr(cronos_tid id, const char *expr);

};


