#include "asgard.hpp"
#include "TriggerJSON.hpp"
#include "Timer.hpp"
#include <torch/script.h> // One-stop header.

/**
 * Taken from the basic examples off the torchscript website
 */

#define ML_MODEL "/home/shining/work/vlite/asgard/ml_modeltraces/battousai9_7K_trace.pt"

#define TEST_DBSON "/home/shining/work/vlite/frbs/p_ngc5033/20200405_052941_muos_ea99_dm212.30_sn11.30_wd06.25.dbson"


void writer (std::array <float, 1024>& arr, const char * fi) {
	std::ofstream ofs(fi);
	std::ostream_iterator<float> osi (std::cout, " ");
	std::copy (arr.cbegin(), arr.cend(), osi);
	ofs.flush ();
}

int main(int argc, const char* argv[]) {
	//if (argc != 2) {
		//std::cout << "usage: DBSON-FILE" << std::endl;
		//return 0;
	//}
	Timer tbtdd ("BTDD");
	Timer tinfer ("Infer");
	Timer treset ("Reset");

	// load the model
	torch::jit::script::Module cnn;
	try {
		// Deserialize the ScriptModule from a file using torch::jit::load().
				cnn = torch::jit::load( ML_MODEL );
	}
	catch (const c10::Error& e) {
		std::cerr << "error loading the model\n";
		return -1;
	}

	// create tensor for the cnn
	// this tensor will be reused for all inferences
	auto topt       = torch::TensorOptions().dtype (torch::kFloat32).requires_grad (true);
	auto t_btdd     = torch::zeros ({1, 2, 32, 32}, topt);
	int decision    = 0;

	// BTDD step
	// 2 * 32 * 32 = 2048
	constexpr float ffac = 1.f / 255.f;
	constexpr float mfac = 1.f / 8.f;
	std::array <float, 1024> ibt = {0.0f};
	std::array <float, 1024> idd = {0.0f};
	DBSON dbson ( TEST_DBSON );
	// bt | 256*256 = 65536
	// DM major
	tbtdd.Start ();
	int idm=0, itt=0, iff=0, idx=0;
	for (unsigned i = 0; i < 65536; i++) {
		idm = ( i / 256 ) / 8;
		itt = ( i % 256 ) / 8;
		idx = idm*32 + itt;
		ibt[idx] += mfac * ffac * dbson.bt[i];
	}
	// dd | 64*256  = 16384
	// Freq major
	for (unsigned i = 0; i < 16384; i++) {
		itt = ( i / 256 ) / 2;
		iff = ( i % 256 ) / 8;
		idx = itt*32 + iff;
		idd[idx] += mfac * ffac * dbson.dd[i];
	}
	tbtdd.StopPrint (std::cout);
	//writer (ibt, "/tmp/i_bt.dat");
	//writer (idd, "/tmp/i_dd.dat");
	tinfer.Start ();
	// INFERENCE step
	{
		auto t_bt = torch::from_blob (ibt.data(), {1, 32, 32}, topt);
		auto t_dd = torch::from_blob (idd.data(), {1, 32, 32}, topt);
		t_btdd.index_put_ ({0,0, torch::indexing::Slice(), torch::indexing::Slice()}, t_bt);
		t_btdd.index_put_ ({0,1, torch::indexing::Slice(), torch::indexing::Slice()}, t_dd );
		std::vector<torch::jit::IValue> infer_this;
		infer_this.push_back (t_btdd);
		auto sfmax  = cnn.forward ( infer_this ).toTensor();
		std::cout << std::endl;
		std::cout << " sfmax " << sfmax << std::endl;
		decision    = sfmax.argmax (1).item().toInt();
	}
	tinfer.StopPrint (std::cout);

	std::cout << "The decision was = " << decision << std::endl;
	treset.Start ();
	decision = 0;
	ibt.fill (0.0f);
	idd.fill (0.0f);
	treset.StopPrint (std::cout);
	return 0;
}
