#include "opl2usb.h"

#include "hidlibrary.h"

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#define BUFF_SIZE 16

struct command_t
{
	command_t() {}

	command_t(char address, char data)
		:address(address)
		, data(data)
	{}

	char address;
	char data;
};

struct dataexchange_t
{
	char size;
	command_t commands[BUFF_SIZE];
};

namespace OPL2USB {
	class CommandsQueue
	{
	public:
		CommandsQueue()
			: th(&CommandsQueue::send_thread, this)
			, hid("dead_man", "OPL2")
		{}

		~CommandsQueue() {
			th.detach();
		}

		void enqueue(command_t cmd)
		{
			std::lock_guard<std::mutex> lock(m);
			q.push(cmd);
			c.notify_one();
		}

		void opl2usb_reset() {
			dataexchange_t data;
			data.size = 1;
			data.commands[0] = command_t(-1, -1);
			hid.SendData(&data);
		}

	protected:
		command_t dequeue()
		{
			std::unique_lock<std::mutex> lock(m);
			while (q.empty())
			{
				// release lock as long as the wait and reaquire it afterwards.
				c.wait(lock);
			}

			command_t val = q.front();
			q.pop();
			return val;
		}

		void send_thread() {
			while (true) {
				dataexchange_t data;
				data.size = 0;

				do
				{
					data.commands[data.size] = dequeue();
					data.size += 1;
				} while (data.size < BUFF_SIZE && q.size() > 0);

				hid.SendData(&data);
			}
		}

		void send_thread2() {
			while (true) {
				dataexchange_t data;
				data.size = 0;

				std::unique_lock<std::mutex> lock(m);
				while (q.empty()) {
					c.wait(lock);
				}

				while (true)
				{
					data.commands[data.size] = q.front();
					data.size += 1;
					q.pop();

					if (data.size == BUFF_SIZE || q.size() == 0)
						break;
				}

				hid.SendData(&data);
			}
		}

	private:
		HIDLibrary<dataexchange_t> hid;
		std::queue<command_t> q;
		mutable std::mutex m;
		std::condition_variable c;
		std::thread th;
	};

	Handler::Handler()
		: opl_commands(new CommandsQueue())
	{
		memset(&buf, 0, sizeof(buf));
	}

	Handler::~Handler() {
		opl_commands->opl2usb_reset();
		delete opl_commands;
	}

	void Handler::WriteReg(Bit32u reg, Bit8u val) {
		opl_commands->enqueue(command_t((char)reg, val));
	}
}
