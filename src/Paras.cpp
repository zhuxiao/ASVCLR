#include <string.h>
#include <vector>
#include <htslib/hts.h>
#include "Paras.h"

// Constructor with parameters
Paras::Paras(){
	init();
}

// Constructor with parameters
Paras::Paras(int argc, char **argv)
{
	init();
	if(parseParas(argc, argv)!=0) exit(1);
}

// initialization
void Paras::init()
{
	command = "";
	inBamFile = "";
	outFilePrefix = "";
	num_threads = 0;

	min_ins_size_filt = 0;
	min_del_size_filt = 0;
	min_clip_size_filt = 0;
	min_ins_num_filt = 0;
	min_del_num_filt = 0;
	min_clip_num_filt = 0;

	reg_sum_size_est = 0;
	max_reg_sum_size_est = MAX_REG_SUM_SIZE_EST;

	canu_version = getCanuVersion();
}

// get the Canu program version
string Paras::getCanuVersion(){
	string canu_version_str, canu_version_cmd, canu_version_filename, line, canu_prog_name;
	ifstream infile;
	vector<string> str_vec;

	canu_version_filename = "canu_version";
	canu_version_cmd = "canu -version > " + canu_version_filename;
	system(canu_version_cmd.c_str());

	infile.open(canu_version_filename);
	if(!infile.is_open()){
		cerr << __func__ << ", line=" << __LINE__ << ": cannot open file " << canu_version_filename << endl;
		exit(1);
	}

	canu_version_str = "";
	getline(infile, line);
	if(line.size()){
		str_vec = split(line, " ");
		canu_prog_name = str_vec.at(0);

		if(canu_prog_name.compare("Canu")==0) canu_version_str = str_vec.at(1);
		else{
			cout << "Canu may be not correctly installed, please check its installation before running this program." << endl;
			exit(1);
		}
	}

	infile.close();

	return canu_version_str;
}

// parse the parameters
int Paras::parseParas(int argc, char **argv)
{
	if (argc < 2) { showUsage(); return 1; }

    if (strcmp(argv[1], "-h") == 0 or strcmp(argv[1], "help") == 0 or strcmp(argv[1], "--help") == 0) {
        if (argc == 2) { showUsage(); return 0; }
        argv++;
        argc = 2;
    }

    if (strcmp(argv[1], "detect")==0){
    	command = "detect";
    	return parseDetectParas(argc-1, argv+1);
    }else if(strcmp(argv[1], "assemble")==0){
    	command = "assemble";
    	return parseAssembleParas(argc-1, argv+1);
    }else if(strcmp(argv[1], "call")==0){
    	command = "call";
    	return parseCallParas(argc-1, argv+1);
    }else if(strcmp(argv[1], "all")==0){
    	command = "all";
    	return parseAllParas(argc-1, argv+1);
    }else{
    	cerr << "invalid command " << argv[1] << endl;
    	showUsage(); return 1;
    }
}

// parse the parameters for detect command
int Paras::parseDetectParas(int argc, char **argv)
{
	int opt, threadNum_tmp = 0, mask_val;
	blockSize = BLOCKSIZE;
	slideSize = SLIDESIZE;
	min_sv_size_usr = MIN_SV_SIZE_USR;
	minClipReadsNumSupportSV = MIN_CLIP_READS_NUM_THRES;
	maxClipRegSize = MAX_CLIP_REG_SIZE;
	mask_val = 1;

	while( (opt = getopt(argc, argv, ":f:b:s:c:o:t:M:h")) != -1 ){
		switch(opt){
			case 'f': refFile = optarg; break;
			case 'b': blockSize = stoi(optarg); break;
			case 's': slideSize = stoi(optarg); break;
			case 'm': min_sv_size_usr = stoi(optarg); break;
			case 'n': minClipReadsNumSupportSV = stoi(optarg); break;
			case 'c': maxClipRegSize = stoi(optarg); break;
			case 'o': outFilePrefix = optarg; break;
			case 't': threadNum_tmp = stoi(optarg); break;
			case 'M': mask_val = stoi(optarg); break;
			case 'h': showDetectUsage(); exit(0);
			case '?': cout << "unknown option -" << (char)optopt << endl; exit(1);
			case ':': cout << "the option -" << (char)optopt << " needs a value" << endl; exit(1);
		}
	}

	load_from_file_flag = true;
	num_threads = (threadNum_tmp>=sysconf(_SC_NPROCESSORS_ONLN)) ? sysconf(_SC_NPROCESSORS_ONLN) : threadNum_tmp;

	if(mask_val==1) maskMisAlnRegFlag = true;
	else if(mask_val==0) maskMisAlnRegFlag = false;
	else{
		cout << "Error: Please specify the correct mask flag for mis-aligned regions using -M option." << endl << endl;
		showDetectUsage();
		return 1;
	}

	opt = argc - optind; // the number of SAMs on the command line
	if(opt==1) inBamFile = argv[optind];
	else { showDetectUsage(); return 1; }

	if(refFile.size()==0){
		cout << "Error: Please specify the reference" << endl << endl;
		showDetectUsage();
		return 1;
	}

	return 0;
}

// parse the parameters for assemble command
int Paras::parseAssembleParas(int argc, char **argv)
{
	int opt, threadNum_tmp = 0, mask_val;
	blockSize = BLOCKSIZE;
	slideSize = ASSEM_SLIDE_SIZE;
	assemSlideSize = ASSEM_SLIDE_SIZE;
	min_sv_size_usr = MIN_SV_SIZE_USR;
	minClipReadsNumSupportSV = MIN_CLIP_READS_NUM_THRES;
	maxClipRegSize = MAX_CLIP_REG_SIZE;
	mask_val = 1;

	while( (opt = getopt(argc, argv, ":f:b:S:m:n:c:o:t:M:h")) != -1 ){
		switch(opt){
			case 'f': refFile = optarg; break;
			case 'b': blockSize = stoi(optarg); break;
			case 'S': assemSlideSize = stoi(optarg); break;
			case 'm': min_sv_size_usr = stoi(optarg); break;
			case 'n': minClipReadsNumSupportSV = stoi(optarg); break;
			case 'c': maxClipRegSize = stoi(optarg); break;
			case 'o': outFilePrefix = optarg; break;
			case 't': threadNum_tmp = stoi(optarg); break;
			case 'M': mask_val = stoi(optarg); break;
			case 'h': showAssembleUsage(); exit(0);
			case '?': cout << "unknown option -" << (char)optopt << endl; exit(1);
			case ':': cout << "the option -" << (char)optopt << " needs a value" << endl; exit(1);
		}
	}

	load_from_file_flag = true;
	num_threads = (threadNum_tmp>=sysconf(_SC_NPROCESSORS_ONLN)) ? sysconf(_SC_NPROCESSORS_ONLN) : threadNum_tmp;

	if(mask_val==1) maskMisAlnRegFlag = true;
	else if(mask_val==0) maskMisAlnRegFlag = false;
	else{
		cout << "Error: Please specify the correct mask flag for mis-aligned regions using -M option." << endl << endl;
		showAssembleUsage();
		return 1;
	}

	opt = argc - optind; // the number of SAMs on the command line
	if(opt==1) inBamFile = argv[optind];
	else { showAssembleUsage(); return 1; }

	if(refFile.size()==0){
		cout << "Error: Please specify the reference" << endl << endl;
		showAssembleUsage();
		return 1;
	}

	return 0;
}

// parse the parameters for call command
int Paras::parseCallParas(int argc, char **argv)
{
	int opt, threadNum_tmp = 0, mask_val;
	blockSize = BLOCKSIZE;
	slideSize = ASSEM_SLIDE_SIZE;
	assemSlideSize = ASSEM_SLIDE_SIZE;
	min_sv_size_usr = MIN_SV_SIZE_USR;
	minClipReadsNumSupportSV = MIN_CLIP_READS_NUM_THRES;
	maxClipRegSize = MAX_CLIP_REG_SIZE;
	mask_val = 1;

	while( (opt = getopt(argc, argv, ":f:b:S:m:n:c:o:t:M:h")) != -1 ){
		switch(opt){
			case 'f': refFile = optarg; break;
			case 'b': blockSize = stoi(optarg); break;
			case 'S': assemSlideSize = stoi(optarg); break;
			case 'm': min_sv_size_usr = stoi(optarg); break;
			case 'n': minClipReadsNumSupportSV = stoi(optarg); break;
			case 'c': maxClipRegSize = stoi(optarg); break;
			case 'o': outFilePrefix = optarg; break;
			case 't': threadNum_tmp = stoi(optarg); break;
			case 'M': mask_val = stoi(optarg); break;
			case 'h': showCallUsage(); exit(0);
			case '?': cout << "unknown option -" << (char)optopt << endl; exit(1);
			case ':': cout << "the option -" << (char)optopt << " needs a value" << endl; exit(1);
		}
	}

	load_from_file_flag = true;
	num_threads = (threadNum_tmp>=sysconf(_SC_NPROCESSORS_ONLN)) ? sysconf(_SC_NPROCESSORS_ONLN) : threadNum_tmp;

	if(mask_val==1) maskMisAlnRegFlag = true;
	else if(mask_val==0) maskMisAlnRegFlag = false;
	else{
		cout << "Error: Please specify the correct mask flag for mis-aligned regions using -M option." << endl << endl;
		showCallUsage();
		return 1;
	}

	opt = argc - optind; // the number of SAMs on the command line
	if(opt==1) inBamFile = argv[optind];
	else { showCallUsage(); return 1; }

	if(refFile.size()==0){
		cout << "Error: Please specify the reference" << endl << endl;
		showCallUsage();
		return 1;
	}

	return 0;
}

// parse the parameters for 'all' command
int Paras::parseAllParas(int argc, char **argv)
{
	int opt, threadNum_tmp = 0, mask_val;
	blockSize = BLOCKSIZE;
	slideSize = SLIDESIZE;
	assemSlideSize = ASSEM_SLIDE_SIZE;
	min_sv_size_usr = MIN_SV_SIZE_USR;
	minClipReadsNumSupportSV = MIN_CLIP_READS_NUM_THRES;
	maxClipRegSize = MAX_CLIP_REG_SIZE;
	mask_val = 1;

	while( (opt = getopt(argc, argv, ":f:b:s:S:m:n:c:o:t:M:h")) != -1 ){
		switch(opt){
			case 'f': refFile = optarg; break;
			case 'b': assemSlideSize = stoi(optarg); break;
			case 's': slideSize = stoi(optarg); break;
			case 'S': assemSlideSize = stoi(optarg); break;
			case 'm': min_sv_size_usr = stoi(optarg); break;
			case 'n': minClipReadsNumSupportSV = stoi(optarg); break;
			case 'c': maxClipRegSize = stoi(optarg); break;
			case 'o': outFilePrefix = optarg; break;
			case 't': threadNum_tmp = stoi(optarg); break;
			case 'M': mask_val = stoi(optarg); break;
			case 'h': showUsage(); exit(0);
			case '?': cout << "unknown option -" << (char)optopt << endl; exit(1);
			case ':': cout << "the option -" << (char)optopt << " needs a value" << endl; exit(1);
		}
	}

	load_from_file_flag = false;
	num_threads = (threadNum_tmp>=sysconf(_SC_NPROCESSORS_ONLN)) ? sysconf(_SC_NPROCESSORS_ONLN) : threadNum_tmp;

	if(mask_val==1) maskMisAlnRegFlag = true;
	else if(mask_val==0) maskMisAlnRegFlag = false;
	else{
		cout << "Error: Please specify the correct mask flag for mis-aligned regions using -M option." << endl << endl;
		showCallUsage();
		return 1;
	}

	opt = argc - optind; // the number of SAMs on the command line
	if(opt==1) inBamFile = argv[optind];
	else { showCallUsage(); return 1; }

	if(refFile.size()==0){
		cout << "Error: Please specify the reference" << endl << endl;
		showCallUsage();
		return 1;
	}

	return 0;
}

// show the usage
void Paras::showUsage()
{
	cout << "Program: asvclr (Accurate Structural Variation Caller for Long Reads)" << endl;
	cout << "Version: 0.1.0 (using htslib " << hts_version() << ")" << endl << endl;
	cout << "Usage:  asvclr  <command> [options]" << endl << endl;

	cout << "Commands:" << endl;
	cout << "     detect       detect indel signatures in aligned reads" << endl;
	cout << "     assemble     assemble candidate regions and align assemblies" << endl;
	cout << "                  back to reference" << endl;
	cout << "     call         call indels by alignments of local genome assemblies" << endl;
	cout << "     all          run the above commands in turn" << endl;
}

// show the usage for detect command
void Paras::showDetectUsage()
{
	cout << "Program: asvclr (Accurate Structural Variation Caller for Long Reads)" << endl;
	cout << "Version: 0.1.0 (using htslib " << hts_version() << ")" << endl << endl;
	cout << "Usage: asvclr detect [options] <in.bam>|<in.sam> [region ...]?" << endl << endl;

	cout << "Options: " << endl;
	cout << "     -f FILE      reference file name (required)" << endl;
	cout << "     -b INT       block size [1000000]" << endl;
	cout << "     -s INT       detect slide size [500]" << endl;
	cout << "     -m INT       minimal SV size to detect [2]" << endl;
	cout << "     -n INT       minimal clipping reads supporting a SV [7]" << endl;
	cout << "     -c INT       maximal clipping region size to detect [10000]" << endl;
	cout << "     -o FILE      prefix of the output file [stdout]" << endl;
	cout << "     -t INT       number of threads [0]" << endl;
	cout << "     -M INT       Mask mis-aligned regions [1]: 1 for yes, 0 for no" << endl;
	cout << "     -h           show this help message and exit" << endl;
}

// show the usage for assemble command
void Paras::showAssembleUsage()
{
	cout << "Program: asvclr (Accurate Structural Variation Caller for Long Reads)" << endl;
	cout << "Version: 0.1.0 (using htslib " << hts_version() << ")" << endl << endl;
	cout << "Usage: asvclr assemble [options] <in.bam>|<in.sam> [region ...]?" << endl << endl;

	cout << "Options: " << endl;
	cout << "     -f FILE      reference file name (required)" << endl;
	cout << "     -b INT       block size [1000000]" << endl;
	cout << "     -S INT       assemble slide size [10000]" << endl;
	cout << "     -c INT       maximal clipping region size to assemble [10000]" << endl;
	cout << "     -o FILE      prefix of the output file [stdout]" << endl;
	cout << "     -t INT       number of threads [0]" << endl;
	cout << "     -M INT       Mask mis-aligned regions [1]: 1 for yes, 0 for no" << endl;
	cout << "     -h           show this help message and exit" << endl;
}

// show the usage for call command
void Paras::showCallUsage()
{
	cout << "Program: asvclr (Accurate Structural Variation Caller for Long Reads)" << endl;
	cout << "Version: 0.1.0 (using htslib " << hts_version() << ")" << endl << endl;
	cout << "Usage: asvclr call [options] <in.bam>|<in.sam> [region ...]?" << endl << endl;

	cout << "Options: " << endl;
	cout << "     -f FILE      reference file name (required)" << endl;
	cout << "     -b INT       block size [1000000]" << endl;
	cout << "     -S INT       assemble slide size [10000]" << endl;
	cout << "     -c INT       maximal clipping region size to call [10000]" << endl;
	cout << "     -o FILE      prefix of the output file [stdout]" << endl;
	cout << "     -t INT       number of threads [0]" << endl;
	cout << "     -M INT       Mask mis-aligned regions [1]: 1 for yes, 0 for no" << endl;
	cout << "     -h           show this help message and exit" << endl;
}

// output parameters
void Paras::outputParas(){
	if(refFile.size()) cout << "Ref file: " << refFile << endl;
	if(inBamFile.size()) cout << "Align file: " << inBamFile << endl;
	if(outFilePrefix.size()) cout << "Out prefix: " << outFilePrefix << endl;
	cout << "Canu version: " << canu_version << endl;

	cout << "Clipping number supporting SV: " << minClipReadsNumSupportSV << endl;
	cout << "Block size: " << blockSize << endl;
	cout << "Slide size: " << slideSize << endl;
	cout << "Maximal clipping region size: " << maxClipRegSize << endl;
	cout << "Num threads: " << num_threads << endl;
	if(maskMisAlnRegFlag) cout << "Mask mis-aligned regions: yes" << endl << endl;
	else cout << "Mask mis-aligned regions: no" << endl << endl;
}

// output the estimation parameters
void Paras::outputEstParas(string info){
	cout << info << endl;
	cout << "min_ins_size_filt: " << min_ins_size_filt << endl;
	cout << "min_del_size_filt: " << min_del_size_filt << endl;
//	cout << "min_clip_size_filt: " << min_clip_size_filt << endl;
	cout << "min_ins_num_filt: " << min_ins_num_filt << endl;
	cout << "min_del_num_filt: " << min_del_num_filt << endl;
	cout << "min_clip_num_filt: " << min_clip_num_filt << endl;
	cout << "large_indel_size_thres: " << large_indel_size_thres << endl << endl;
}

// initialize the estimation auxiliary data
void Paras::initEst(){
	size_t i;
	for(i=0; i<AUX_ARR_SIZE; i++) insSizeEstArr[i] = 0;
	for(i=0; i<AUX_ARR_SIZE; i++) delSizeEstArr[i] = 0;
	for(i=0; i<AUX_ARR_SIZE; i++) clipSizeEstArr[i] = 0;
	for(i=0; i<AUX_ARR_SIZE; i++) insNumEstArr[i] = 0;
	for(i=0; i<AUX_ARR_SIZE; i++) delNumEstArr[i] = 0;
	for(i=0; i<AUX_ARR_SIZE; i++) clipNumEstArr[i] = 0;
}

// estimate parameters
void Paras::estimate(size_t op_est){
	if(op_est==SIZE_EST_OP){
		// min_ins_size_filt
//		cout << "min_ins_size_filt:" << endl;
		min_ins_size_filt = estimateSinglePara(insSizeEstArr, AUX_ARR_SIZE, SIZE_PERCENTILE_EST, MIN_INDEL_EVENT_SIZE);

		// min_del_size_filt
//		cout << "min_del_size_filt:" << endl;
		min_del_size_filt = estimateSinglePara(delSizeEstArr, AUX_ARR_SIZE, SIZE_PERCENTILE_EST, MIN_INDEL_EVENT_SIZE);

		// min_clip_size_filt
//		min_clip_size_filt = estimateSinglePara(clipSizeEstArr, AUX_ARR_SIZE, SIZE_PERCENTILE_EST, MIN_INDEL_EVENT_SIZE);

		if(min_ins_size_filt>min_del_size_filt) large_indel_size_thres = min_ins_size_filt * LARGE_INDEL_SIZE_FACTOR;
		else large_indel_size_thres = min_del_size_filt * LARGE_INDEL_SIZE_FACTOR;

	}else if(op_est==NUM_EST_OP){
		// min_ins_num_filt
//		cout << "min_ins_num_filt:" << endl;
		min_ins_num_filt = estimateSinglePara(insNumEstArr, AUX_ARR_SIZE, NUM_PERCENTILE_EST, MIN_INDEL_EVENT_NUM);

		// min_del_num_filt
//		cout << "min_del_num_filt:" << endl;
		min_del_num_filt = estimateSinglePara(delNumEstArr, AUX_ARR_SIZE, NUM_PERCENTILE_EST, MIN_INDEL_EVENT_NUM);

		// min_clip_num_filt
//		cout << "min_clip_num_filt:" << endl;
		min_clip_num_filt = estimateSinglePara(clipNumEstArr, AUX_ARR_SIZE, NUM_PERCENTILE_EST, MIN_INDEL_EVENT_NUM);
	}else if(op_est==SNV_EST_OP){

	}else{
		cerr << __func__ << ", line=" << __LINE__ << ", invalid estimation op_flag: " << op_est << endl;
		exit(1);
	}
}

// estimate single parameter
size_t Paras::estimateSinglePara(size_t *arr, size_t n, double threshold, size_t min_val){
	size_t i, total, val = 1;
	double total1;

	for(i=0, total=0; i<AUX_ARR_SIZE; i++) total += arr[i];
	if(total>0)
//		cout << "total=" << total << endl;
		for(i=0, total1=0; i<AUX_ARR_SIZE; i++){
			total1 += arr[i];
//			cout << "\t" << i << ": " << arr[i] << ", " << total1/total << endl;
			if(total1/total>=threshold){
				val = i + 1;
				break;
			}
		}
	if(val<min_val) val = min_val;

	return val;
}

// string split function
vector<string> Paras::split(const string& s, const string& delim)
{
    vector<string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len)
    {
        int find_pos = s.find(delim, pos);
        if (find_pos < 0)
        {
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

