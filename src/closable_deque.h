#include <deque>
#include <string>
#include <sstream>
#include <iostream>
#include <mutex>
#include <condition_variable>

#pragma once

class closable_deque: public std::streambuf {
private:
    std::deque<std::string> data_;
    bool closed_;
    mutable std::mutex mutex_;
    mutable std::condition_variable cv_;
    std::vector<char> buffer_;

public:
    closable_deque() : closed_(false) {
        buffer_.resize(1024);
        setp(buffer_.data(), buffer_.data() + buffer_.size());
    }
    
    ~closable_deque() {
        close();
    }

    // 禁止拷贝，但允许移动
    closable_deque(const closable_deque&) = delete;
    closable_deque& operator=(const closable_deque&) = delete;
    
    closable_deque(closable_deque&& other) noexcept {
        std::lock_guard<std::mutex> lock(other.mutex_);
        data_ = std::move(other.data_);
        closed_ = other.closed_;
        buffer_ = std::move(other.buffer_);
        other.closed_ = true;
    }
    
    closable_deque& operator=(closable_deque&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::mutex> lock1(mutex_, std::defer_lock);
            std::unique_lock<std::mutex> lock2(other.mutex_, std::defer_lock);
            std::lock(lock1, lock2);
            
            data_ = std::move(other.data_);
            closed_ = other.closed_;
            buffer_ = std::move(other.buffer_);
            other.closed_ = true;
        }
        return *this;
    }

    // std::queue 兼容接口
    void push(const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) return;
        data_.push_back(value);
        cv_.notify_one();
    }

    void push(std::string&& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) return;
        data_.push_back(std::move(value));
        cv_.notify_one();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.size();
    }

    std::string front() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (data_.empty()) {
            throw std::runtime_error("closable_deque is empty");
        }
        return data_.front();
    }

    void pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!data_.empty()) {
            data_.pop_front();
        }
    }

    // 非阻塞的尝试获取
    bool try_pop(std::string& result) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (data_.empty()) {
            return false;
        }
        result = std::move(data_.front());
        data_.pop_front();
        return true;
    }

    // 阻塞等待直到有数据或关闭
    bool wait_and_pop(std::string& result, std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this] { return !data_.empty() || closed_; });

        if (!data_.empty()) {
            result = std::move(data_.front());
            data_.pop_front();
            return true;
        }

        return false; // timeout 或者已关闭且无数据
    }

    // closable_deque 特有接口
    void close() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
        cv_.notify_all();
    }

    bool is_closed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }

    bool is_finished() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_ && data_.empty();
    }

    // std::streambuf

    int sync() override {
        // 获取写入的数据
        char* start = pbase();  // 缓冲区开始位置
        char* end = pptr();     // 当前写入位置
        std::ptrdiff_t length = end - start;  // 写入的数据长度

        if (length > 0) {
            // 处理写入的数据
            std::string written_data(start, length);

            push(written_data);
            
            // 重置写入指针到缓冲区开始
            setp(buffer_.data(), buffer_.data() + buffer_.size());
        }

        return 0;
    }
};