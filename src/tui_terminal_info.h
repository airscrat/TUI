#pragma once
#include <string>
#include <vector>

namespace tui 
{
	struct key_seq_pair
	{
		std::string name;
		std::vector<int> seq;
		key_seq_pair() {}
		key_seq_pair(std::string Name) : name(Name) {}
		key_seq_pair(std::string Name, std::vector<int> Seq) : name(Name), seq(Seq) {}
	};

	struct terminal_info
	{
		std::vector<key_seq_pair> sequences =
		{
			{"kpp"}, //PGUP
			{"knp"}, //PGDN
			{"kdch1"}, //DEL
			{"kich1"}, //INS
			{"kend"}, //END
			{"khome"}, //HOME

			{"kf1"}, //F1
			{"kf2"},
			{"kf3"},
			{"kf4"},
			{"kf5"},
			{"kf6"},
			{"kf7"},
			{"kf8"},
			{"kf9"},
			{"kf10"},
			{"kf11"},
			{"kf12"}, //F12

			{"kcuu1"}, //UP
			{"kcud1"}, //DOWN
			{"kcub1"}, //LEFT
			{"kcuf1"}, //RIGHT

			//ALT KEYS
			{"kPRV3"}, //ALT+PGUP
			{"kNXT3"}, //ALT+PGDN
			{"kDC3"}, //ALT+DEL
			{"kIC3"}, //ALT+INS
			{"kEND3"}, //ALT+END
			{"kHOM3"}, //ALT+HOME

			{"kUP3"}, //ALT+UP
			{"kDN3"}, //ALT+DOWN
			{"kLFT3"}, //ALT+LEFT
			{"kRIT3"} //ALT+RIGHT
		};
		unsigned short longest_seq;
		unsigned short shortest_seq;

		std::string smkx;
		std::string rmkx;

		bool rgb_color;

		terminal_info() { set(); }

		//load terminal information
		void set();

		//return position of sequence, if there is no given sequence return -1
		int getSeqNumber(const std::vector<int>& seq);

		//return sequence
		std::vector<int> getSeq(unsigned int seq);
	};

	extern terminal_info term_info;
}
