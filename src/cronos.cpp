/*                                                                                                                                                            
PZEM EDL - PZEM Event Driven Library                                                                                                                          
                                                                                                                                                              
This code implements communication and data exchange with PZEM004T V3.0 module using MODBUS proto                                                             
and provides an API for energy metrics monitoring and data processing.                                                                                        
                                                                                                                                                              
This file is part of the 'PZEM event-driven library' project.                                                                                                 
                                                                                                                                                              
Copyright (C) Emil Muratov, 2021                                                                                                                              
GitHub: https://github.com/vortigont/pzem-edl                                                                                                                 
*/
#include "cronos.hpp"
#include <ctime>
//#include "Arduino.h"

#define DEFAULT_RESCHEDULING_TIME   60    // seconds

static constexpr const char* tag = "CronoS";

CronoS_Task::CronoS_Task(const char* expression){
  setExpr(expression);
}

void CronoS_Task::setExpr(const char* expr){
  const char* err;
  cron_parse_expr(expr, &rule, &err);
  valid = (err == NULL);
/*
  if (!valid){
    ESP_LOGW(tag, "Task expr invalid: %s\n", err);
  }
*/
}


cronos_tid CronoS::addCallback(const char* expression, CronoS_Callback_t cb, void* arg){
  std::lock_guard<std::mutex> lock(_mtx);
  _tasks.emplace_back(std::make_unique<CronoS_Callback>(expression, cb, arg));
  cronos_tid id = ++_cnt;
  _tasks.back()->_id = id;
  std::time_t now;
  std::time(&now);
  _tasks.back()->next_run = cron_next(&_tasks.back()->rule, now);
  if (_tmr && xTimerIsTimerActive(_tmr))
    xTimerChangePeriod(_tmr, 1, portMAX_DELAY);

  return id;
}


CronoS::~CronoS(){
  if (_tmr){
    xTimerStop( _tmr, portMAX_DELAY );
    xTimerDelete( _tmr, portMAX_DELAY );
    _tmr = nullptr;
  }
}


void CronoS::start(){
  if (!_tmr){
    _tmr = xTimerCreate(tag,
                    1,        // we start with a 1 tick timer, it will recalculate on next run
                    pdTRUE,
                    static_cast<void*>(this),
                    [](TimerHandle_t h) { static_cast<CronoS*>(pvTimerGetTimerID(h))->_evaluate(); }
                  );
  } else {
    xTimerChangePeriod(_tmr, 1, portMAX_DELAY);
  }

  xTimerReset( _tmr, portMAX_DELAY );
}

void CronoS::stop(){
  if (_tmr)
    xTimerStop( _tmr, portMAX_DELAY );
}

void CronoS::clear(){
  std::lock_guard<std::mutex> lock(_mtx);
  stop();
  _tasks.clear();
};

void CronoS::_evaluate(){
  if (!_tasks.size()){
    // disable timer when no tasks are present
    stop();
    return;
  }

  std::time_t now;    // by default reevaluate tasks at least once in a minute
  std::time(&now);
  std::time_t earliest(now + DEFAULT_RESCHEDULING_TIME);

  std::lock_guard<std::mutex> lock(_mtx);
  for (auto i = _tasks.begin(); i != _tasks.end(); ++i){
    // skip malformed/disabled tasks
    if (!(*i)->valid){
      continue;
    }

    if (now == (*i)->next_run || now > (*i)->next_run){
      (*i)->cronos_run();
      (*i)->next_run = cron_next(&(*i)->rule, now);
      // since some task has just runned, let's give a chance to a scheduler to go with another threads before we continue with next one
      // this is to not create a congestion when multiple tasks should run at the same time
      xTimerChangePeriod(_tmr, 1, portMAX_DELAY);
      xTimerReset( _tmr, portMAX_DELAY );
      return;
    }

    earliest = std::min(earliest, (*i)->next_run);
    //ESP_LOGV(tag,"Next event in %u sec\n", (*i)->next_run - now);
  }

  uint32_t awake = 1000 * (earliest - now);

  //ESP_LOGI(tag, "Sleep for: %u\n", awake);
  //Serial.printf("Sleep for: %u\n", awake);

  xTimerChangePeriod(_tmr, pdMS_TO_TICKS(awake), portMAX_DELAY);
  xTimerReset( _tmr, portMAX_DELAY );
}

void CronoS::removeTask(cronos_tid id){
  std::lock_guard<std::mutex> lock(_mtx);
  for (auto i = _tasks.begin(); i != _tasks.end(); ++i){
    if (i->get()->_id == id){
      _tasks.erase(i);
      return;
    }
  }
}

int CronoS::getCrontab(cronos_tid id, char *buffer, int buffer_len, int expr_len, const char **error) const {
  //std::lock_guard<std::mutex> lock(_mtx);
  for (auto i = _tasks.begin(); i != _tasks.end(); ++i){
    if (i->get()->_id == id){
      return cron_generate_expr(&((*i)->rule), buffer, buffer_len, expr_len, error);
    }
  }

  return -1;
}

void CronoS::setExpr(cronos_tid id, const char *expr){
  for (auto &t : _tasks ){
    if (t->getID() == id){
      t->setExpr(expr);
      return;
    }
  }
}
