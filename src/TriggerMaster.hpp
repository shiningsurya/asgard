#pragma once
#include <asgard.hpp>
#include <exception>
#include "Dedisperser.hpp"
// Header stuff
#include <Header.hpp>


class TriggerMaster {
	private:
		DedispManager ddm;
		// headers
		Header_t  head;
		trigger_t trig;

	public:
		void DumpHead (const Header_t& head_, const trigger_t& trig_ ) {
			head = head_;
			trig = trig_;
		}



};
