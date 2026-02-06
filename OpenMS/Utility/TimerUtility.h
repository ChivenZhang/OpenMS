#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com & gemini
*
* =================================================*/
#include "MS.h"
#include <asio.hpp>

/// @brief Timer Utility
class TimerUtility
{
public:
    // 将任务内部状态封装，以便在 TimerId 和时间轮之间共享
    struct TaskState
    {
        std::chrono::steady_clock::time_point next_expiry;
        uint64_t repeat_ms;
        std::function<void()> callback;
        int rotation;        // 补全：记录任务还需要轮转多少圈
        bool cancelled = false;

        TaskState(std::chrono::steady_clock::time_point first, uint64_t rep, std::function<void()> cb)
            : next_expiry(first), repeat_ms(rep), callback(std::move(cb)), rotation(0)
        {
        }
    };

    // 现在 TimerId 持有指向状态的弱引用，安全且支持运行期判断
    using TimerId = std::weak_ptr<TaskState>;

    /// @brief Constructs and initializes a TimerUtility: sets up the internal periodic timer, stores the tick interval and slot count, resizes the timing wheel, and starts ticking.
    /// @param slots Number of slots in the timing wheel; determines the wheel's capacity and scheduling granularity.
    /// @param interval Tick interval as a std::chrono::milliseconds duration; used to initialize the timer and stored internally (in milliseconds).
    TimerUtility(size_t slots = 60, std::chrono::milliseconds interval = std::chrono::milliseconds(10))
        :
        interval_ms_(interval.count()),
        slots_(slots),
        current_pos_(0),
        current_tick_count_(0),
        start_time_(std::chrono::steady_clock::now()),
        timer_(io_, interval) // 注意顺序：timer 需要在 io_ 之后
    {
        wheel_.resize(slots_);

        manager_thread_ = std::thread([this]()
        {
            tick();
            io_.run();
        });
    }

    ~TimerUtility()
    {
        // 1. 先取消定时器，触发 async_wait 的回调链断裂，确保 run() 能尽快退出
        asio::post(io_, [this]() {
            asio::error_code ec;
            timer_.cancel(ec);
        });

        // 2. 必须先 Join，确保 manager_thread_ 不再访问任何成员变量
        if (manager_thread_.joinable())
        {
            manager_thread_.join();
        }
    }

    /// @brief Schedules a task to run after a specified timeout and, optionally, repeatedly using a timing wheel. The function computes the slot index and rotation based on the internal interval and slots, stores the task state, and returns an identifier for the scheduled timer.
    /// @param timeout_ms Initial delay before the task is first executed, in milliseconds.
    /// @param repeat_ms Repeat interval in milliseconds. If zero, the timer is one-shot; if non-zero, the task will be rescheduled with this period.
    /// @param task Callable (std::function<void()>) to run when the timer fires. The callable is moved into the timer's internal state.
    /// @return A TimerId identifying the scheduled timer (typically a weak reference to the internal task state). Use this identifier to cancel or manage the timer; the returned object does not itself keep the task alive if the implementation stores only weak references.
    TimerId start(uint64_t timeout_ms, uint64_t repeat_ms, std::function<void()> task)
    {
        auto first = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
        auto state = std::make_shared<TaskState>(first, repeat_ms, std::move(task));

        // 如果此时 io_context 还没运行或者已经停止，post 可能无效
        asio::post(io_, [this, state]() {
            schedule_task_internal(state);
            });
        return state;
    }

    /// @brief Stops the timer referred to by the given TimerId. If the timer state is available, marks it as cancelled and clears its callback to release associated resources.
    /// @param id A handle (typically a weak reference) to the timer's shared state. The function attempts to lock it; if the state is not available (expired/invalid), no action is taken.
    void stop(TimerId id)
    {
        asio::post(io_, [=, this]()
        {
            if (auto state = id.lock())
            {
                state->cancelled = true;
                state->callback = nullptr; // 释放回调持有的资源
            }
        });
    }

private:
    void schedule_task_internal(std::shared_ptr<TaskState> state)
    {
        auto duration = state->next_expiry - start_time_;
        int64_t total_ticks = duration.count() / (interval_ms_ * 1000000);

        size_t index = total_ticks % slots_;
        state->rotation = (total_ticks - current_tick_count_) / (int)slots_;
        if (state->rotation < 0) state->rotation = 0;

        wheel_[index].push_back(state);
    }

    void tick()
    {
        // 这一步产生的异步等待是维持 io_context 不退出的唯一动力
        timer_.async_wait([this](const asio::error_code& ec) {
            // 如果 ec 是 operation_aborted（手动取消），则不再递归，异步链断裂，run() 结束
            if (ec) return;

            auto& bucket = wheel_[current_pos_];
            for (auto it = bucket.begin(); it != bucket.end(); ) {
                auto state = *it;
                if (state->cancelled) {
                    it = bucket.erase(it);
                    continue;
                }

                if (state->rotation > 0) {
                    state->rotation--;
                    ++it;
                }
                else {
                    if (state->callback) {
                        try { state->callback(); }
                        catch (...) {}
                    }

                    it = bucket.erase(it);
                    if (!state->cancelled && state->repeat_ms > 0) {
                        state->next_expiry += std::chrono::milliseconds(state->repeat_ms);
                        schedule_task_internal(state);
                    }
                }
            }

            current_pos_ = (current_pos_ + 1) % slots_;
            current_tick_count_++;

            timer_.expires_at(start_time_ + std::chrono::milliseconds((current_tick_count_ + 1) * interval_ms_));
            tick(); // 递归挂载，维持异步链
            });
    }

private:
    asio::io_context io_;           // 最先构造，最后析构
    uint64_t interval_ms_;
    size_t slots_;
    size_t current_pos_;
    int64_t current_tick_count_;
    std::chrono::steady_clock::time_point start_time_;
    std::vector<std::list<std::shared_ptr<TaskState>>> wheel_;
    asio::steady_timer timer_;
    std::thread manager_thread_;    // 最后构造，析构时最先执行 join
};