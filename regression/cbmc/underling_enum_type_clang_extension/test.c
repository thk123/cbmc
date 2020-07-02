typedef enum : unsigned { X, Y } unsigned_enum;

void __CPROVER_assert(_Bool condition, const char* msg);

int test(unsigned_enum value) {
	int underlying_value = (int)value;
	__CPROVER_assert(underlying_value >= 0, "Enum underling type must be unsigned");
	return 0;
}
