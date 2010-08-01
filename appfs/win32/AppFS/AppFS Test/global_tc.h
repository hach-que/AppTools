// Utility functions for the truncate section of the test suite.
bool tc_verify(const char* filename, int intended_size);
void tc_scale(uint32_t max);
void tc_setup();
void tc_setto(uint32_t current, uint32_t max, uint32_t iter_current, uint32_t iter_max);