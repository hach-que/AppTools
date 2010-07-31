int test_inspect(AppLib::LowLevel::FS * fs, uint16_t inodeid);
int test_next_id(AppLib::LowLevel::FS * fs, uint16_t & inodeid);
int test_file_unique(AppLib::LowLevel::FS * fs, const char* filename);
int test_allocate_node(AppLib::LowLevel::FS * fs, uint16_t inodeid, const char* filename);
int test_add_child_to_dir(AppLib::LowLevel::FS * fs, uint16_t child, uint16_t parent);
int test_rw_all_0_var(AppLib::LowLevel::FS * fs, uint16_t inodeid, std::string & test_data);
int test_rw_burst_30_492(AppLib::LowLevel::FS * fs, uint16_t inodeid, std::string & test_data);
int test_rw_stream1_20_512(AppLib::LowLevel::FS * fs, uint16_t inodeid, std::string & test_data);
int test_rw_stream10_0_1000(AppLib::LowLevel::FS * fs, uint16_t inodeid);