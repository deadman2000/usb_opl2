#include "adlib.h"

namespace OPL2USB {

	class CommandsQueue;

	struct Handler : public Adlib::Handler {
		Handler();

		~Handler();

		virtual void WriteReg(Bit32u reg, Bit8u val);

		virtual Bit32u WriteAddr(Bit32u port, Bit8u val) {
			return val;
		}

		virtual void Generate(MixerChannel* chan, Bitu samples) {
			while (samples > 0) {
				Bitu todo = samples > 1024 ? 1024 : samples;
				samples -= todo;
				chan->AddSamples_m16(todo, buf);
			}
		}

		virtual void Init(Bitu rate) {}

	private:
		Bit16s buf[1024];
		CommandsQueue * opl_commands;
	};
}
