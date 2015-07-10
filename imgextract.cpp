#include <getopt.h>
#include <string>
#include <iostream>
#include <map>
#include <sys/stat.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TKey.h>
#include <TROOT.h>
#include <TF1.h>

#include <TASImage.h>
#include <TString.h>

#include <TApplication.h>
#include <TStyle.h>
#include <TSystem.h>

#include <RootTools.h>

#include <iostream>
#define PR(x) std::cout << "++DEBUG: " << #x << " = |" << x << "| (" << __FILE__ << ", " << __LINE__ << ")\n";

int flag_png;
int flag_eps;
int flag_pdf;

int flag_width = 800;
int flag_height = 600;

std::string outpath = "./";

TDirectory * target;

bool has_filter = false;
std::map<std::string, int> filter_map;

TString filter = "";

unsigned int counter = 0;

struct CanvasCfg
{
	std::string cn;
	int w;
	int h;
};

inline bool file_exists(const std::string & file)
{
	struct stat buffer;
	return (stat(file.c_str(), &buffer) == 0);
}

void exportimg(TObject * obj, TDirectory * dir)
{
	TCanvas * can = nullptr;

	if (has_filter and filter_map.find(obj->GetName()) == filter_map.end())
		return;
	//	continue;

	dir->GetObject(obj->GetName(), can);

	can->Draw();
	can->SetCanvasSize(flag_width, flag_height);
	std::cout << "Exporting " << can->GetName() << std::endl;

	if (flag_png) RootTools::ExportPNG((TCanvas*)can, outpath);
	if (flag_eps) RootTools::ExportEPS((TCanvas*)can, outpath);
	if (flag_pdf) RootTools::ExportPDF((TCanvas*)can, outpath);
	++counter;
}

void browseDir(TDirectory * dir, std::vector<std::string> * filters = nullptr)
{
	TKey * key;
	TIter nextkey(dir->GetListOfKeys());
	while ((key = (TKey*)nextkey()))
	{
		TObject *obj = key->ReadObj();
// 		PR("MemSrc");gDirectory->ls("-d");
		if (obj->InheritsFrom("TDirectory")) {
			TDirectory * dir = (TDirectory*)obj;
			browseDir(dir);
		} else
		if (obj->InheritsFrom("TCanvas")) {
			exportimg(obj, dir);
		}
	}
}

bool extractor(const std::string & file) {
	TFile * f = TFile::Open(file.c_str(), "READ");

	if (!f->IsOpen())
	{
		std::cerr << "File " << file << " not open!" << std::endl;
		return false;
	}

	TSystem sys;

	std::string dir_name = sys.DirName(file.c_str());
	std::string file_basename = sys.BaseName(file.c_str());

	size_t last_dot = file_basename.find_last_of('.');
// 	size_t ext_len = file_basename.end() - last_dot;
	std::string imgcfg_name = dir_name + "/." + file_basename.replace(last_dot, std::string::npos, ".iecfg");
	PR(imgcfg_name);

	if (file_exists(imgcfg_name))
	{
	}

	f->cd();

	browseDir(f);

	std::cout << "Total: " << counter << std::endl;
	return true;
}

int main(int argc, char ** argv) {
	TROOT AnalysisDST_Cal1("TreeAnalysis","compiled analysisDST macros");
	TApplication app("treeanal", NULL, NULL, NULL, 0);
	gROOT->SetBatch();

	struct option lopt[] =
		{
			{"png",			no_argument,		&flag_png,	1},
			{"eps",			no_argument,		&flag_eps,	1},
			{"pdf",			no_argument,		&flag_pdf,	1},
			{"dir",			required_argument,	0,		'd'},
			{"width",		required_argument,	0,		'w'},
			{"height",		required_argument,	0,		'h'},
			{"filter",		required_argument,	0,		'f'},
			{ 0, 0, 0, 0 }
		};


	Int_t c = 0;
	while (1) {
		int option_index = 0;

		c = getopt_long(argc, argv, "d:w:h:f:", lopt, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 0:
				if (lopt[option_index].flag != 0)
					break;
				printf ("option %s", lopt[option_index].name);
				if (optarg)
					printf (" with arg %s", optarg);
				printf ("\n");
				break;
			case 'd':
				outpath = optarg;
				break;
			case 'w':
				flag_width = atoi(optarg);
				break;
			case 'h':
				flag_height = atoi(optarg);
				break;
			case 'f':
				has_filter = true;
				++filter_map[optarg];
				break;
			case '?':
// 				Usage();
				exit(EXIT_SUCCESS);
				break;
//			case '?':
// 				abort();
//				break;
			default:
				break;
		}
	}

	target = gDirectory;

	RootTools::NicePalette();
	RootTools::MyMath();

	std::cout << "Filtering for :" << std::endl;
	for (std::map<std::string,int>::iterator it = filter_map.begin(); it != filter_map.end(); ++it)
		std::cout << " " << it->first << std::endl;

	while (optind < argc)
	{
		std::cout << "Extracting from " << argv[optind] << std::endl;
		extractor(argv[optind++]);
	}

	return 0;
}
