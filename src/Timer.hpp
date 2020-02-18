#pragma once
#include <chrono>

namespace ch  = std::chrono;

class Timer {
  using clk = std::chrono::high_resolution_clock;
  using string_t = std::string;
  private:
    string_t               sid;
    ch::time_point<clk>    start;
    ch::time_point<clk>    stop;
    float                dur;
    float                dur_sec;
    float                dur_msec;

  public:
    Timer ( string_t _s = string_t("unamed") ) : sid(_s) {}
    ~Timer () = default;

    void Start();
    void Stop();
    void StopPrint(std::ostream&);

    friend std::ostream& operator<< (std::ostream& os, const Timer& t);
};

void Timer::Start () {
  start = clk::now ();
}

void Timer::Stop () {
  stop = clk::now ();
  // clock 
  auto sclk_dur = ch::duration_cast<ch::seconds>(stop - start);
  auto mclk_dur = ch::duration_cast<ch::milliseconds>(stop - start);
  //ticks
  dur_sec  = sclk_dur.count ();
  dur_msec = mclk_dur.count ();
  dur = dur_sec + (dur_msec/1000.0);
}

std::ostream& operator<< (std::ostream& os, const Timer& t) {
  os << "Timer::" << t.sid;
  os << " ";
  os << t.dur << std::endl;; 
  return os;
}

void Timer::StopPrint(std::ostream& os) {
  Stop ();
  os << *this;
}
