#pragma once
#include "asgard.hpp"
#include <torch/script.h> // One-stop header.

#include "TriggerJSON.hpp"
#include "Timer.hpp"

#define ML_MODEL "/home/vlite-master/surya/asgard/ml_modeltraces/battousai9_7K_trace.pt"
#define ML_MODEL2 "/home/vlite-master/surya/asgard/ml_modeltraces/retraced_after_pilot.pt"


/**
 * A lot of the sizes and shapes have been coded in.
 * Will have to change it when we change the shapes.
 * */
namespace tj = torch::jit;
class TriggerCNN  {
  using ce = c10::Error;
	using Byte = unsigned char;
	using vf   = std::vector<float>;
	using vb   = std::vector<Byte>;

  private:
    tj::script::Module      cnn;
    torch::TensorOptions    topt;
    torch::Tensor           t_btdd;
    std::array<float, 1024> ibt;
    std::array<float, 1024> idd;

    bool load_model () {
      try {
        cnn = tj::load ( ML_MODEL );
      }
      catch ( ce& e) {
        std::cerr << "TriggerCNN::Load_Model failed!" << std::endl;
        return false;
      }
      return true;
    }

    void clear_arrays () {
      ibt.fill (0.0f);
      idd.fill (0.0f);
      t_btdd.fill_ (0.0f);
    }

    static constexpr float norm_fac = 1.f / 255.f ;
    static constexpr float bt_fac    = norm_fac / 64.f;
    static constexpr float dd_fac    = norm_fac / 16.f;

  public:
    TriggerCNN () {
      // prepare tensors
      topt      = torch::TensorOptions().dtype (torch::kFloat32).requires_grad (false);
      t_btdd    = torch::zeros ({1, 2, 32, 32}, topt);
      // load model
      load_model ();
    }

    template<typename T>
    bool Inference ( std::vector<T>& bt, std::vector<T>& dd ) {
      clear_arrays ();
      int idm=0, itt=0, iff=0, idx=0;
      int decision=0;
      // bt | 256*256 = 65536
      // DM major
      for (unsigned i = 0; i < 65536; i++) {
        idm = ( i / 256 ) / 8;
        itt = ( i % 256 ) / 8;
        idx = idm*32 + itt;
        //idx = itt*32 + idm;
        ibt[idx] += (bt_fac * bt[i]);
      }
      // dd | 64*256  = 16384
      // Freq major
      for (unsigned i = 0; i < 16384; i++) {
        itt = ( i / 64 ) / 8;
        iff = ( i % 64 ) / 2;
        idx = itt*32 + iff;
        idd[idx] += (dd_fac * dd[i]);
      }
      // INFERENCE
      {
        auto t_bt = torch::from_blob (ibt.data(), {1, 32, 32}, topt);
        auto t_dd = torch::from_blob (idd.data(), {1, 32, 32}, topt);
        t_btdd.index_put_ ({0,0, torch::indexing::Slice(), torch::indexing::Slice()}, t_bt);
        t_btdd.index_put_ ({0,1, torch::indexing::Slice(), torch::indexing::Slice()}, t_dd );
        std::vector<torch::jit::IValue> infer_this;
        infer_this.push_back (t_btdd);
        auto sfmax  = cnn.forward ( infer_this ).toTensor();
        decision    = sfmax.argmax (1).item().toInt();
        std::cout << "TriggerCNN::CNN_out" << sfmax << std::endl;
      }
      return decision == 1;
    }

};
