#pragma once

#include <atomic>

template<typename T>
struct node
{
	T data_;
	std::atomic<node<T>*> next_;

	explicit node(const T& data)
	: data_(data)
	, next_(nullptr)
	{ }
};

template<typename T>
class lfQueue
{
	std::atomic<node<T>*> head_;
	std::atomic<node<T>*> tail_;

public:
	lfQueue()
	{
		// Инициализируем с dummy-узлом
		node<T>* dummy = new node<T>(T {});
		head_.store(dummy, std::memory_order_relaxed);
		tail_.store(dummy, std::memory_order_relaxed);
	}

	~lfQueue()
	{
		// Очистка памяти (в реальном коде нужно аккуратно обрабатывать многопоточность)
		while (node<T>* current = head_.load())
		{
			head_.store(current->next);
			delete current;
		}
	}

	void push(const T& data)
	{
		node<T>* new_node = new node<T>(data);

		while (true)
		{
			node<T>* current_tail = tail_.load(std::memory_order_acquire);
			node<T>* next = current_tail->next.load(std::memory_order_acquire);

			// Проверяем, что tail не изменился
			if (current_tail != tail_.load(std::memory_order_relaxed))
			{
				continue;
			}

			if (next == nullptr)
			{
				// Пытаемся добавить new_node в конец
				if (current_tail->next.compare_exchange_weak(next, new_node, std::memory_order_release, std::memory_order_relaxed))
				{
					// Успешно добавили, обновляем tail
					tail_.compare_exchange_strong(current_tail, new_node, std::memory_order_release, std::memory_order_relaxed);
					break;
				}
			}
			else
			{
				// Помогаем другим потокам продвинуть tail
				tail_.compare_exchange_strong(current_tail, next, std::memory_order_release, std::memory_order_relaxed);
			}
		}
	}

	bool pop(T& result)
	{
		node<T>* current_head = nullptr;
		node<T>* current_tail = nullptr;
		node<T>* next = nullptr;

		while (true)
		{
			current_head = head_.load(std::memory_order_acquire);
			current_tail = tail_.load(std::memory_order_acquire);
			next = current_head->next.load(std::memory_order_acquire);

			if (current_head == head_.load(std::memory_order_acquire))
			{
				if (current_head == current_tail)
				{
					if (next == nullptr)
					{
						return false; // Queue is empty
					}
					tail_.compare_exchange_weak(current_tail, next, std::memory_order_release, std::memory_order_relaxed);
				}
				else
				{
					result = next->data_;
					if (head_.compare_exchange_weak(current_head, next, std::memory_order_release, std::memory_order_relaxed))
					{
						delete current_head;
						return true;
					}
				}
			}
		}
	}
};