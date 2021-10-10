#pragma once

#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

namespace thread_safe
{
	template <typename T>
	class queue
	{
	private:

		mutable std::mutex mtx;
		std::queue<T> data;
		std::condition_variable cond;

	public:

		queue() {}
		queue(const queue& other)
		{
			std::lock_guard<std::mutex> lock(other.mtx);
			data = other.data;
		}

		void push(const T& val)
		{
			std::lock_guard<std::mutex> lock(mtx);

			data.push(val);
			cond.notify_one();
		}

		void wait_and_pop(T& val)
		{
			std::unique_lock<std::mutex> lock(mtx);

			cond.wait(lock, [this]() { return !data.empty(); });
			val = data.front();
			data.pop();
		}

		std::shared_ptr<T> wait_and_pop()
		{
			std::unique_lock<std::mutex> lock(mtx);

			cond.wait(lock, [this]() { return !data.empty(); });
			std::shared_ptr<T> res(std::make_shared<T>(data.front()));
			data.pop();

			return res;
		}

		bool try_pop(T& val)
		{
			std::lock_guard<std::mutex> lock(mtx);

			if (data.empty())
				return false;

			val = data.front();
			data.pop();

			return true;
		}

		T try_pop()
		{
			std::lock_guard<std::mutex> lock(mtx);

			if (data.empty())
				return {};

			auto res = data.front();
			data.pop();

			return res;
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lock(mtx);
			return data.empty();
		}
	};
}

#endif