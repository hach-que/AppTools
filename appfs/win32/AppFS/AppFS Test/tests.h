// Low-level tests and functions.
int test_ll_inspect(AppLib::LowLevel::FS * fs, uint16_t inodeid);
int test_ll_next_id(AppLib::LowLevel::FS * fs, uint16_t & inodeid);
int test_ll_file_unique(AppLib::LowLevel::FS * fs, const char* filename);
int test_ll_allocate_node(AppLib::LowLevel::FS * fs, uint16_t inodeid, const char* filename);
int test_ll_add_child_to_dir(AppLib::LowLevel::FS * fs, uint16_t child, uint16_t parent);

// Read-write tests.
int test_rw_all_0_var(AppLib::LowLevel::FS * fs, uint16_t inodeid, std::string & test_data);
int test_rw_burst_30_492(AppLib::LowLevel::FS * fs, uint16_t inodeid, std::string & test_data);
int test_rw_stream1_20_512(AppLib::LowLevel::FS * fs, uint16_t inodeid, std::string & test_data);
int test_rw_stream10_0_1000(AppLib::LowLevel::FS * fs, uint16_t inodeid);
int test_rw_stream1_0_1024(AppLib::LowLevel::FS * fs, uint16_t inodeid);

// FuseLink operation tests.
int test_op_create(AppLib::LowLevel::FS * fs, const char* filename);

// Truncation tests.
int test_tc_incdec_400(AppLib::LowLevel::FS * fs, uint32_t inodeid, const char* path);
int test_tc_incdec_4000(AppLib::LowLevel::FS * fs, uint32_t inodeid, const char* path);
int test_tc_incdec_4000000(AppLib::LowLevel::FS * fs, uint32_t inodeid, const char* path);
int test_tc_rand_400(AppLib::LowLevel::FS * fs, uint32_t inodeid, const char* path);

// Bug detection tests.
int test_bg_boundary_205(AppLib::LowLevel::FS * fs, uint32_t inodeid, const char* path);