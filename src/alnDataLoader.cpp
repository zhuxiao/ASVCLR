#include "alnDataLoader.h"

extern pthread_mutex_t mutex_down_sample;
//alnDataLoader::alnDataLoader(){
//}

alnDataLoader::alnDataLoader(string &chrname, int32_t startRefPos, int32_t endRefPos, string &inBamFile, int32_t minMapQ) {
	this->reg_str = chrname + ":" + to_string(startRefPos) + "-" + to_string(endRefPos);
	this->inBamFile = inBamFile;
	mean_read_len = 0;
	this->minMapQ = minMapQ;
	this->startRefPos = startRefPos;
	this->endRefPos = endRefPos;
}

alnDataLoader::~alnDataLoader() {
}

//void alnDataLoader::loadAlnData(vector<bam1_t*> &alnDataVector){
//	samFile *in = NULL;
//	bam_hdr_t *header;
//
//	if ((in = sam_open(inBamFile.c_str(), "r")) == 0) {
//		cerr << __func__ << ": failed to open " << inBamFile << " for reading" << endl;
//		exit(1);
//	}
//
//	if ((header = sam_hdr_read(in)) == 0) {
//		cerr << __func__ << ": fail to read the header from " << inBamFile << endl;
//		exit(1);
//	}
//
//	hts_idx_t *idx = sam_index_load(in, inBamFile.c_str()); // load index
//	if (idx == 0) { // index is unavailable
//		cerr << __func__ << ": random alignment retrieval only works for indexed BAM files.\n" << endl;
//		exit(1);
//	}
//
//	hts_itr_t *iter = sam_itr_querys(idx, header, reg_str.c_str()); // parse a region in the format like `chr2:100-200'
//	if (iter == NULL) { // region invalid or reference name not found
//		int beg, end;
//		if (hts_parse_reg(reg_str.c_str(), &beg, &end))
//			cerr <<  __func__ << ": region " << reg_str << " specifies an unknown reference name." << endl;
//		else
//			cerr <<  __func__ << ": region " << reg_str << " could not be parsed." << endl;
//		exit(1);
//	}
//
//	// load align data from iteration
//	loadAlnDataFromIter(alnDataVector, in, header, iter, reg_str);
//	hts_itr_destroy(iter);
//	hts_idx_destroy(idx); // destroy the BAM index
//	bam_hdr_destroy(header);
//	if(in) sam_close(in);
//}

void alnDataLoader::loadAlnData(vector<bam1_t*> &alnDataVector, double max_ultra_high_cov){
	samFile *in = NULL;
	bam_hdr_t *header;
	vector<int32_t> qlen_vec;
	size_t total_len=0, total_num=0;

	if ((in = sam_open(inBamFile.c_str(), "r")) == 0) {
		cerr << __func__ << ": failed to open " << inBamFile << " for reading" << endl;
		exit(1);
	}

	if ((header = sam_hdr_read(in)) == 0) {
		cerr << __func__ << ": fail to read the header from " << inBamFile << endl;
		exit(1);
	}

	hts_idx_t *idx = sam_index_load(in, inBamFile.c_str()); // load index
	if (idx == 0) { // index is unavailable
		cerr << __func__ << ": random alignment retrieval only works for indexed BAM files.\n" << endl;
		exit(1);
	}

	hts_itr_t *iter = sam_itr_querys(idx, header, reg_str.c_str()); // parse a region in the format like `chr2:100-200'
	if (iter == NULL) { // region invalid or reference name not found
		int beg1, end1;
		if (hts_parse_reg(reg_str.c_str(), &beg1, &end1))
			cerr <<  __func__ << ": region " << reg_str << " specifies an unknown reference name." << endl;
		else
			cerr <<  __func__ << ": region " << reg_str << " could not be parsed." << endl;
		exit(1);
	}
	computeAlnDataNumFromIter(in, header, iter, reg_str, qlen_vec, total_len, total_num); // total_num, total_len, q_len
	//cout << "total_len="<< total_len << " total_num=" << total_num << " q_len.size()=" << qlen_vec.size() << endl;

	hts_itr_destroy(iter);

	iter = sam_itr_querys(idx, header, reg_str.c_str());
	if (iter == NULL) { // region invalid or reference name not found
		int beg2, end2;
		if (hts_parse_reg(reg_str.c_str(), &beg2, &end2))
			cerr <<  __func__ << ": region " << reg_str << " specifies an unknown reference name." << endl;
		else
			cerr <<  __func__ << ": region " << reg_str << " could not be parsed." << endl;
		exit(1);
	}

	// load align data from iteration
//	loadAlnDataFromIter(alnDataVector, in, header, iter, reg_str);
	loadAlnDataFromIter(alnDataVector, in, header, iter, reg_str, max_ultra_high_cov, qlen_vec, total_len, total_num);

	hts_itr_destroy(iter);
	hts_idx_destroy(idx); // destroy the BAM index
	bam_hdr_destroy(header);
	if(in) sam_close(in);
}

void alnDataLoader::loadAlnData(vector<bam1_t*> &alnDataVector, double max_ultra_high_cov, vector<string> &target_qname_vec){
	samFile *in = NULL;
	bam_hdr_t *header;
	vector<string> qname_vec;
	vector<int32_t> qlen_vec;
	size_t total_len=0, total_num=0;

	if ((in = sam_open(inBamFile.c_str(), "r")) == 0) {
		cerr << __func__ << ": failed to open " << inBamFile << " for reading" << endl;
		exit(1);
	}

	if ((header = sam_hdr_read(in)) == 0) {
		cerr << __func__ << ": fail to read the header from " << inBamFile << endl;
		exit(1);
	}

	hts_idx_t *idx = sam_index_load(in, inBamFile.c_str()); // load index
	if (idx == 0) { // index is unavailable
		cerr << __func__ << ": random alignment retrieval only works for indexed BAM files.\n" << endl;
		exit(1);
	}

	hts_itr_t *iter = sam_itr_querys(idx, header, reg_str.c_str()); // parse a region in the format like `chr2:100-200'
	if (iter == NULL) { // region invalid or reference name not found
		int beg1, end1;
		if (hts_parse_reg(reg_str.c_str(), &beg1, &end1))
			cerr <<  __func__ << ": region " << reg_str << " specifies an unknown reference name." << endl;
		else
			cerr <<  __func__ << ": region " << reg_str << " could not be parsed." << endl;
		exit(1);
	}
	computeAlnDataNumFromIter(in, header, iter, reg_str, qname_vec, qlen_vec, total_len, total_num); // total_num, total_len, q_len
	//cout << "total_len="<< total_len << " total_num=" << total_num << " q_len.size()=" << qlen_vec.size() << endl;

	hts_itr_destroy(iter);

	iter = sam_itr_querys(idx, header, reg_str.c_str());
	if (iter == NULL) { // region invalid or reference name not found
		int beg2, end2;
		if (hts_parse_reg(reg_str.c_str(), &beg2, &end2))
			cerr <<  __func__ << ": region " << reg_str << " specifies an unknown reference name." << endl;
		else
			cerr <<  __func__ << ": region " << reg_str << " could not be parsed." << endl;
		exit(1);
	}

	// load align data from iteration
	loadAlnDataFromIter(alnDataVector, in, header, iter, reg_str, max_ultra_high_cov, target_qname_vec, qname_vec, qlen_vec, total_len, total_num);

	hts_itr_destroy(iter);
	hts_idx_destroy(idx); // destroy the BAM index
	bam_hdr_destroy(header);
	if(in) sam_close(in);

}

void alnDataLoader::loadAlnData(vector<bam1_t*> &alnDataVector, vector<string> &target_qname_vec){
	samFile *in = NULL;
	bam_hdr_t *header;

	if ((in = sam_open(inBamFile.c_str(), "r")) == 0) {
		cerr << __func__ << ": failed to open " << inBamFile << " for reading" << endl;
		exit(1);
	}

	if ((header = sam_hdr_read(in)) == 0) {
		cerr << __func__ << ": fail to read the header from " << inBamFile << endl;
		exit(1);
	}

	hts_idx_t *idx = sam_index_load(in, inBamFile.c_str()); // load index
	if (idx == 0) { // index is unavailable
		cerr << __func__ << ": random alignment retrieval only works for indexed BAM files.\n" << endl;
		exit(1);
	}

	hts_itr_t *iter = sam_itr_querys(idx, header, reg_str.c_str()); // parse a region in the format like `chr2:100-200'
	if (iter == NULL) { // region invalid or reference name not found
		int beg1, end1;
		if (hts_parse_reg(reg_str.c_str(), &beg1, &end1))
			cerr <<  __func__ << ": region " << reg_str << " specifies an unknown reference name." << endl;
		else
			cerr <<  __func__ << ": region " << reg_str << " could not be parsed." << endl;
		exit(1);
	}

	// load align data from iteration
	loadAlnDataFromIter(alnDataVector, in, header, iter, reg_str, target_qname_vec);

	hts_itr_destroy(iter);
	hts_idx_destroy(idx); // destroy the BAM index
	bam_hdr_destroy(header);
	if(in) sam_close(in);

}

// compute align data number from iteration
void alnDataLoader::computeAlnDataNumFromIter(samFile *in, bam_hdr_t *header, hts_itr_t *iter, string& reg, vector<int32_t> &qlen_vec, size_t &total_len, size_t &total_num){
	int result;
	bam1_t *b;
	b = bam_init1();
	while ((result = sam_itr_next(in, iter, b)) >= 0) {
		if(b->core.l_qseq>0 and (b->core.qual>=minMapQ and b->core.qual!=255)){
			total_len += b->core.l_qseq;
			total_num++;
			qlen_vec.push_back(b->core.l_qseq);
		}
		bam_destroy1(b);
		b = bam_init1();
	}
	mean_read_len = (double) total_len / total_num;

//	cout << "total_len=" << total_len << " ;total_num=" << total_num << " ;mean_read_len=" << mean_read_len << endl;
	bam_destroy1(b);

	if (result < -1) {
		cerr <<  __func__ << ": retrieval of region " << reg << " failed due to truncated file or corrupt BAM index file." << endl;
		exit(1);
	}
}

// compute align data number from iteration
void alnDataLoader::computeAlnDataNumFromIter(samFile *in, bam_hdr_t *header, hts_itr_t *iter, string& reg, vector<string> &qname_vec, vector<int32_t> &qlen_vec, size_t &total_len, size_t &total_num){
	int result;
	bam1_t *b;
	string qname;

	b = bam_init1();
	while ((result = sam_itr_next(in, iter, b)) >= 0) {
		if(b->core.l_qseq>0 and (b->core.qual>=minMapQ and b->core.qual!=255)){
			total_len += b->core.l_qseq;
			total_num++;
			qname = bam_get_qname(b);
			qname_vec.push_back(qname);
			qlen_vec.push_back(b->core.l_qseq);
		}
		bam_destroy1(b);
		b = bam_init1();
	}
	mean_read_len = (double) total_len / total_num;

//	cout << "total_len=" << total_len << " ;total_num=" << total_num << " ;mean_read_len=" << mean_read_len << endl;
	bam_destroy1(b);

	if (result < -1) {
		cerr <<  __func__ << ": retrieval of region " << reg << " failed due to truncated file or corrupt BAM index file." << endl;
		exit(1);
	}
}


// load align data from iteration
//void alnDataLoader::loadAlnDataFromIter(vector<bam1_t*> &alnDataVector, samFile *in, bam_hdr_t *header, hts_itr_t *iter, string& reg){
//	int result;
//	size_t sum, count;
//	bam1_t *b;
//
//	// fetch alignments
//	sum = count = 0;
//	b = bam_init1();
//	while ((result = sam_itr_next(in, iter, b)) >= 0) {
//		if(b->core.l_qseq>0 and (b->core.qual>=minMapQ and b->core.qual!=255)){
//		//if(b->core.qual>=minMapQ and b->core.qual!=255){
//			sum += b->core.l_qseq;
//			count++;
//			alnDataVector.push_back(b);
//		}else bam_destroy1(b);
//		b = bam_init1();
//	}
//	mean_read_len = (double) sum / count;
//
//	alnDataVector.shrink_to_fit();
//	bam_destroy1(b);
//	if (result < -1) {
//		cerr <<  __func__ << ": retrieval of region " << reg << " failed due to truncated file or corrupt BAM index file." << endl;
//		exit(1);
//	}
//}

void alnDataLoader::loadAlnDataFromIter(vector<bam1_t*> &alnDataVector, samFile *in, bam_hdr_t *header, hts_itr_t *iter, string& reg, double max_ultra_high_cov, vector<int32_t> &qlen_vec, size_t total_len, size_t total_num){
	int result, i;
	bam1_t *b;
	double expected_total_bases;//, sampled_cov;
	double compensation_coefficient, local_cov_original;
	size_t index, reg_size, reads_count, max_reads_num, total_bases;
	int8_t *selected_flag_array;

	selected_flag_array = (int8_t*) calloc(qlen_vec.size(), sizeof(int8_t));
//	selected_flag_array = (int8_t*) calloc(alnDataVector.size(), sizeof(int8_t));
	if(selected_flag_array==NULL){
		cerr << __func__ << ", line=" << __LINE__ << ": cannot allocate memory, error!" << endl;
		exit(1);
	}

	compensation_coefficient = computeCompensationCoefficient(startRefPos, endRefPos);
	local_cov_original = computeLocalCov(total_len, compensation_coefficient);
	if(max_ultra_high_cov>0 and local_cov_original>max_ultra_high_cov){  // down sample
		reg_size = endRefPos - startRefPos + 1 + mean_read_len;
		expected_total_bases = reg_size * max_ultra_high_cov;
		max_reads_num = qlen_vec.size();
		// make sure each down-sampling is equivalent
		pthread_mutex_lock(&mutex_down_sample);
		srand(1);
		reads_count = 0;
		total_bases = 0;

		while(total_bases <= expected_total_bases and reads_count <= max_reads_num){
			index = rand() % max_reads_num;
			if(selected_flag_array[index]==0){
				selected_flag_array[index] = 1;
				total_bases += qlen_vec.at(index);
				reads_count ++;
//				cout << "index=" << index << " selected_flag_array[index]=" << selected_flag_array[index] << " total_bases=" << total_bases << " reads_count=" << reads_count << endl;
			}
		}
		pthread_mutex_unlock(&mutex_down_sample);
		//sampled_cov = total_bases / reg_size;
		//cout << "sampled_cov=" << sampled_cov << endl;

		b = bam_init1();
		i = 0;
		while((result = sam_itr_next(in, iter, b)) >= 0) {
			if(b->core.l_qseq>0 and (b->core.qual>=minMapQ and b->core.qual!=255)){
				if(selected_flag_array[i] == 1) alnDataVector.push_back(b);
				else bam_destroy1(b);
				i++;
			}else bam_destroy1(b);
			b = bam_init1();
		}
		free(selected_flag_array);
	}else{ // load all data
		b = bam_init1();
		while ((result = sam_itr_next(in, iter, b)) >= 0) {
			if(b->core.l_qseq>0 and (b->core.qual>=minMapQ and b->core.qual!=255)) alnDataVector.push_back(b);
			else bam_destroy1(b);
			b = bam_init1();
		}
	}

	bam_destroy1(b);
	if (result < -1) {
		cerr <<  __func__ << ": retrieval of region " << reg << " failed due to truncated file or corrupt BAM index file." << endl;
		exit(1);
	}
	alnDataVector.shrink_to_fit();
}

void alnDataLoader::loadAlnDataFromIter(vector<bam1_t*> &alnDataVector, samFile *in, bam_hdr_t *header, hts_itr_t *iter, string& reg, double max_ultra_high_cov, vector<string> &target_qname_vec, vector<string> &qname_vec, vector<int32_t> &qlen_vec, size_t total_len, size_t total_num){
	int result;
	bam1_t *b;
	double expected_total_bases;//, sampled_cov;
	double compensation_coefficient, local_cov_original;
	size_t i, index, reg_size, reads_count, max_reads_num, total_bases;
	int8_t *selected_flag_array;
	string qname;

	if(qname_vec.size()!=qlen_vec.size()){
		cerr << __func__ << ", line=" << __LINE__ << ": qname_vec.size=" << qname_vec.size() << ", qlen_vec.size=" << qlen_vec.size() << ", error!" << endl;
		exit(1);
	}

	selected_flag_array = (int8_t*) calloc(qlen_vec.size(), sizeof(int8_t));
//	selected_flag_array = (int8_t*) calloc(alnDataVector.size(), sizeof(int8_t));
	if(selected_flag_array==NULL){
		cerr << __func__ << ", line=" << __LINE__ << ": cannot allocate memory, error!" << endl;
		exit(1);
	}

	compensation_coefficient = computeCompensationCoefficient(startRefPos, endRefPos);
	local_cov_original = computeLocalCov(total_len, compensation_coefficient);
	if(max_ultra_high_cov>0 and local_cov_original>max_ultra_high_cov){  // down sample
		reg_size = endRefPos - startRefPos + 1 + mean_read_len;
		expected_total_bases = reg_size * max_ultra_high_cov;
		max_reads_num = qlen_vec.size();

		// mark the target reads
		reads_count = 0;
		total_bases = 0;
		for(i=0; i<max_reads_num; i++){
			qname = qname_vec.at(i);
			if(find(target_qname_vec.begin(), target_qname_vec.end(), qname) != target_qname_vec.end()){  // found
				selected_flag_array[i] = 1;
				total_bases += qlen_vec.at(i);
				reads_count ++;
			}
		}

		// make sure each down-sampling is equivalent
		pthread_mutex_lock(&mutex_down_sample);
		srand(1);
		while(total_bases <= expected_total_bases and reads_count <= max_reads_num){
			index = rand() % max_reads_num;
			if(selected_flag_array[index]==0){
				selected_flag_array[index] = 1;
				total_bases += qlen_vec.at(index);
				reads_count ++;
//				cout << "index=" << index << " selected_flag_array[index]=" << selected_flag_array[index] << " total_bases=" << total_bases << " reads_count=" << reads_count << endl;
			}
		}
		pthread_mutex_unlock(&mutex_down_sample);
		//sampled_cov = total_bases / reg_size;
		//cout << "sampled_cov=" << sampled_cov << endl;

		b = bam_init1();
		i = 0;
		while((result = sam_itr_next(in, iter, b)) >= 0) {
			if(b->core.l_qseq>0 and (b->core.qual>=minMapQ and b->core.qual!=255)){
				if(selected_flag_array[i] == 1) alnDataVector.push_back(b);
				else bam_destroy1(b);
				i++;
			}else bam_destroy1(b);
			b = bam_init1();
		}
		free(selected_flag_array);
	}else{ // load all data
		b = bam_init1();
		while ((result = sam_itr_next(in, iter, b)) >= 0) {
			if(b->core.l_qseq>0 and (b->core.qual>=minMapQ and b->core.qual!=255)) alnDataVector.push_back(b);
			else bam_destroy1(b);
			b = bam_init1();
		}
	}

	bam_destroy1(b);
	if (result < -1) {
		cerr <<  __func__ << ": retrieval of region " << reg << " failed due to truncated file or corrupt BAM index file." << endl;
		exit(1);
	}
	alnDataVector.shrink_to_fit();
}


void alnDataLoader::loadAlnDataFromIter(vector<bam1_t*> &alnDataVector, samFile *in, bam_hdr_t *header, hts_itr_t *iter, string& reg, vector<string> &qname_vec){
	int result;
	size_t sum, count;
	bam1_t *b;
	string qname;
	bool flag;

	// fetch alignments
	sum = count = 0;
	b = bam_init1();
	while ((result = sam_itr_next(in, iter, b)) >= 0) {
		flag = false;
		if(b->core.l_qseq>0 and (b->core.qual>=minMapQ and b->core.qual!=255)){
			qname = bam_get_qname(b);
			if(find(qname_vec.begin(), qname_vec.end(), qname) != qname_vec.end()){  // found
				sum += b->core.l_qseq;
				count++;
				alnDataVector.push_back(b);
				flag = true;
			}
		}
		if(flag == false) bam_destroy1(b);
		b = bam_init1();
	}
	mean_read_len = (double) sum / count;

	alnDataVector.shrink_to_fit();
	bam_destroy1(b);
	if (result < -1) {
		cerr <<  __func__ << ": retrieval of region " << reg << " failed due to truncated file or corrupt BAM index file." << endl;
		exit(1);
	}
}


double alnDataLoader::computeLocalCov(size_t total_len, double compensation_coefficient){
	double cov = 0;
//	bam1_t *b;
	size_t refined_reg_size;

	refined_reg_size = endRefPos - startRefPos + 1 + 2 * mean_read_len;
	if(refined_reg_size>0){
		cov = total_len / refined_reg_size * compensation_coefficient;
		//cout << "total bases: " << total << " bp, local coverage: " << cov << endl;
	}else{
		cov = -1;
		//cerr << "ERR: ref_seq_size=" << ref_seq_size << endl;
	}
	return cov;
}

double alnDataLoader::computeCompensationCoefficient(size_t startRefPos, size_t endRefPos){
	size_t total_reg_size, refined_reg_size, reg_size_cns;
	double comp_coefficient;

	reg_size_cns = endRefPos - startRefPos + 1;
	total_reg_size = reg_size_cns + 2 * mean_read_len;
	refined_reg_size = reg_size_cns + mean_read_len;  // flanking_area = (2 * mean_read_len) / 2
	comp_coefficient = (double)total_reg_size / refined_reg_size;

	return comp_coefficient;
}

// release the memory
void alnDataLoader::freeAlnData(vector<bam1_t*> &alnDataVector){
	if(!alnDataVector.empty()){
		vector<bam1_t*>::iterator aln;
		for(aln=alnDataVector.begin(); aln!=alnDataVector.end(); aln++)
			bam_destroy1(*aln);
		vector<bam1_t*>().swap(alnDataVector);
		mean_read_len = 0;
	}
}
